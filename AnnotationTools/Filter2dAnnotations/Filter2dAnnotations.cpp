// EvaluateAnnotations.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <cutil_inline.h>
#include "GlobalDefines.h"
#include "MatrixConversion.h"
#include "../common/json.h"
#include "../common/Segmentation.h"
#include "../common/Aggregation.h"
#include "LabelUtil.h"

extern "C" void convertDepthFloatToCameraSpaceFloat4(float4* d_output, float* d_input, float4x4 intrinsicsInv, unsigned int width, unsigned int height);
extern "C" void computeNormals(float4* d_output, float4* d_input, unsigned int width, unsigned int height);
extern "C" void resampleFloatMap(float* d_colorMapResampledFloat, unsigned int outputWidth, unsigned int outputHeight,
	float* d_colorMapFloat, unsigned int inputWidth, unsigned int inputHeight);
extern "C" void resampleUCharMap(unsigned char* d_MapResampled, unsigned int outputWidth, unsigned int outputHeight,
	unsigned char* d_Map, unsigned int inputWidth, unsigned int inputHeight);
extern "C" void gaussFilterFloat4Map(float4* d_output, float4* d_input, float sigmaD, float sigmaR, unsigned int width, unsigned int height);
extern "C" void bilateralFilterFloatMap(float* d_output, float* d_input, float sigmaD, float sigmaR, unsigned int width, unsigned int height);
extern "C" void bilateralFilterFloat4Map(float4* d_output, float4* d_input, float sigmaD, float sigmaR, unsigned int width, unsigned int height);
extern "C" void filterAnnotations(unsigned char* d_outputInstance, const unsigned char* d_inputInstance,
	const float* d_depth, const float* d_intensity, const unsigned char* d_instanceToIdx,
	const unsigned char* d_idxToInstance, float* d_vote,
	int structureSize, int width, int height, float sigmaD, float sigmaR, float intensityScale);
extern "C" void convertInstanceToLabel(unsigned short* d_outputLabel, const unsigned char* d_inputInstance,
	const unsigned short* d_instanceToLabel, unsigned int width, unsigned int height);

struct FilterData {
	FilterData() {
		d_instanceToIdx = NULL;
		d_idxToInstance = NULL;
		d_vote = NULL; 
		d_instanceToLabel = NULL;
		d_depth = NULL;
		d_intensity = NULL;
		d_instance = NULL;
		d_instanceHelper = NULL; 
		d_label = NULL;
		d_depthHelper = NULL;
		d_intensityHelper = NULL;
	}
	void alloc(unsigned int width, unsigned int height) {
		MLIB_CUDA_SAFE_CALL(cudaMalloc(&d_instanceToIdx, sizeof(unsigned char)*MAX_NUM_LABELS_PER_SCENE));
		MLIB_CUDA_SAFE_CALL(cudaMalloc(&d_idxToInstance, sizeof(unsigned char)*MAX_NUM_LABELS_PER_SCENE));
		MLIB_CUDA_SAFE_CALL(cudaMalloc(&d_instanceToLabel, sizeof(unsigned short)*MAX_NUM_LABELS_PER_SCENE));
		MLIB_CUDA_SAFE_CALL(cudaMalloc(&d_vote, sizeof(float)*width*height*MAX_NUM_LABELS_PER_SCENE));
		MLIB_CUDA_SAFE_CALL(cudaMalloc(&d_depth, sizeof(float)*width*height));
		MLIB_CUDA_SAFE_CALL(cudaMalloc(&d_intensity, sizeof(float)*width*height));
		MLIB_CUDA_SAFE_CALL(cudaMalloc(&d_depthHelper, sizeof(float)*width*height));
		MLIB_CUDA_SAFE_CALL(cudaMalloc(&d_intensityHelper, sizeof(float)*width*height));
		MLIB_CUDA_SAFE_CALL(cudaMalloc(&d_instance, sizeof(unsigned char)*width*height));
		MLIB_CUDA_SAFE_CALL(cudaMalloc(&d_instanceHelper, sizeof(unsigned char)*width*height));
		MLIB_CUDA_SAFE_CALL(cudaMalloc(&d_label, sizeof(unsigned short)*width*height));
	}
	void init(const std::vector<unsigned char>& objectIdsToIdx, const std::vector<unsigned char>& idxToObjectIds, const std::vector<unsigned short>& objectIdsToLabel) {
		//initialize
		MLIB_CUDA_SAFE_CALL(cudaMemcpy(d_instanceToIdx, objectIdsToIdx.data(), sizeof(unsigned char)*MAX_NUM_LABELS_PER_SCENE, cudaMemcpyHostToDevice));
		MLIB_CUDA_SAFE_CALL(cudaMemcpy(d_idxToInstance, idxToObjectIds.data(), sizeof(unsigned char)*MAX_NUM_LABELS_PER_SCENE, cudaMemcpyHostToDevice));
		MLIB_CUDA_SAFE_CALL(cudaMemcpy(d_instanceToLabel, objectIdsToLabel.data(), sizeof(unsigned short)*MAX_NUM_LABELS_PER_SCENE, cudaMemcpyHostToDevice));
	}
	void free() {
		MLIB_CUDA_SAFE_FREE(d_instanceToIdx);
		MLIB_CUDA_SAFE_FREE(d_idxToInstance);
		MLIB_CUDA_SAFE_FREE(d_instanceToLabel);
		MLIB_CUDA_SAFE_FREE(d_vote);
		MLIB_CUDA_SAFE_FREE(d_depth);
		MLIB_CUDA_SAFE_FREE(d_intensity);
		MLIB_CUDA_SAFE_FREE(d_depthHelper);
		MLIB_CUDA_SAFE_FREE(d_intensityHelper);
		MLIB_CUDA_SAFE_FREE(d_instance);
		MLIB_CUDA_SAFE_FREE(d_instanceHelper);
		MLIB_CUDA_SAFE_FREE(d_label);
	}

	unsigned char* d_instanceToIdx, *d_idxToInstance; float* d_vote; unsigned short* d_instanceToLabel;
	float* d_depth, *d_intensity; unsigned char* d_instance, *d_instanceHelper; unsigned short* d_label;
	float* d_depthHelper, *d_intensityHelper;
};

std::vector<std::string> readScenesFromFile(const std::string& filename);
void process(FilterData& filterData, std::string path, std::string scanNetPath, std::string outputPath, const std::string& name, bool printDebugOutput);
void processSegs(std::string path, std::string scanNetPath, std::string outputPath, const std::string& name);

void debugVis(const std::string& rawPath, const std::string& filtPath, const std::string& scene, const std::string& scanNetPath, const std::string& outDir);

int _tmain(int argc, _TCHAR* argv[])
{
	try {
		//-------Fill in the paths accordingly here
		const bool bPrintDebugOutput = false;
		const std::string scanNetDir = "../../data/scans/";
		const std::string dataPath = "../annotations-2d/";
		const std::string outputPath = "../annotations-2d-filtered/";
		const std::string sceneListFile = "../../Tasks/Benchmark/scannet_train.txt";
		LabelUtil::get().init("../../data/tasks/scannet-labels.combined.tsv");
		//-------

		if (!util::directoryExists(scanNetDir) || !util::directoryExists(dataPath))
			throw MLIB_EXCEPTION("input data dir(s) do not exist");
		if (!sceneListFile.empty() && !util::fileExists(sceneListFile))
			throw MLIB_EXCEPTION("scene list file (" + sceneListFile + ") does not exist");

		if (!util::directoryExists(outputPath)) util::makeDirectory(outputPath);

		Directory annotationDir(dataPath);
		std::vector<std::string> scenes;
		if (sceneListFile.empty())
			scenes = annotationDir.getDirectories();
		else
			scenes = readScenesFromFile(sceneListFile);

		std::cout << "found " << scenes.size() << " scenes" << std::endl;
		unsigned int counter = 0;
		FilterData filterData; filterData.alloc(1296, 968); //max width/height
		for (const std::string& scene : scenes) {
			if (!util::directoryExists(dataPath + scene)) continue;
			Timer t;
			process(filterData, dataPath + scene, scanNetDir, outputPath + scene, scene, bPrintDebugOutput);
			t.stop(); std::cout << "[" << counter << " | " << scenes.size() << "] time for scene: " << t.getElapsedTime() << " s" << std::endl;
			counter++;
		}
		filterData.free();
		std::cout << std::endl << "processed " << counter << " scenes" << std::endl;
	}
	catch (MLibException& e)
	{
		std::stringstream ss;
		ss << "exception caught:" << e.what() << std::endl;
		std::cout << ss.str() << std::endl;
	}
	std::cout << "done done done" << std::endl;
	//getchar();
	return 0;
}

std::vector<std::string> readScenesFromFile(const std::string& filename)
{
	std::vector<std::string> scenes;
	std::ifstream s(filename); std::string line;
	while (std::getline(s, line))
		scenes.push_back(line);
	return scenes;
}

void visualizeAnnotations(const std::string& prefix, const BaseImage<unsigned char>& instanceImage, const BaseImage<unsigned short>& labelImage = BaseImage<unsigned short>(),
	bool printLegend = false, const Aggregation& agg = Aggregation())
{
	static std::unordered_map<unsigned short, vec3uc> colormap;
	for (const auto& p : instanceImage) {
		if (p.value != 0 && colormap.find(p.value) == colormap.end()) {
			RGBColor c = RGBColor::randomColor();
			while (vec3f(c).length() < 0.05f)
				c = RGBColor::randomColor();
			colormap[p.value] = vec3uc(c.x, c.y, c.z);
		}
	}
	if (labelImage.getNumPixels() > 0) {
		for (const auto& p : labelImage) {
			if (p.value != 0 && colormap.find(p.value) == colormap.end()) {
				RGBColor c = RGBColor::randomColor();
				while (vec3f(c).length() < 0.05f)
					c = RGBColor::randomColor();
				colormap[p.value] = vec3uc(c.x, c.y, c.z);
			}
		}
	}
	ColorImageR8G8B8 visInstance(instanceImage.getDimensions()), visLabel(labelImage.getDimensions());
	for (unsigned int y = 0; y < instanceImage.getHeight(); y++) {
		for (unsigned int x = 0; x < instanceImage.getWidth(); x++) {
			const auto instance = instanceImage(x, y);
			if (instance == 0) visInstance(x, y) = vec3uc(0, 0, 0);
			else visInstance(x, y) = colormap[instance];
			if (labelImage.getNumPixels() > 0) {
				const auto label = labelImage(x, y);
				if (label == 0) visLabel(x, y) = vec3uc(0, 0, 0);
				else visLabel(x, y) = colormap[label];
			}
		}
	}
	FreeImageWrapper::saveImage(prefix + "-instance.png", visInstance);
	if (labelImage.getNumPixels() > 0) FreeImageWrapper::saveImage(prefix + "-label.png", visLabel);

	if (printLegend) {
		std::unordered_map<unsigned char, std::pair<std::string, vec3uc>> instanceColors;
		std::unordered_map<unsigned short, std::pair<std::string, vec3uc>> labelColors;
		const auto& objectIdToLabels = agg.getObjectIdsToLabels();
		bool bRaw = util::endsWith(prefix, "raw");
		for (const auto& p : instanceImage) {
			if (p.value != 0) {
				const unsigned char inst = p.value - 1;
				const auto it = instanceColors.find(inst);
				if (it == instanceColors.end()) {
					const auto it2 = objectIdToLabels.find(inst);
					instanceColors[inst] = std::make_pair(it2->second, colormap[inst+1]);
					if (labelImage(p.x, p.y) > 0) {
						std::string labelName;
						bool valid = LabelUtil::get().getLabelForId(labelImage(p.x, p.y), labelName);
						MLIB_ASSERT(valid);
						labelColors[labelImage(p.x, p.y)] = std::make_pair(labelName, colormap[labelImage(p.x, p.y)]);

						if (bRaw && labelName != it2->second)
							std::cout << "ERROR: inconsistent instance/label" << std::endl;
					}
				}
			}
		}
		const std::string legendPrefix = prefix + "_legend-";
		for (const auto& c : instanceColors) {
			ColorImageR8G8B8 image(100, 100);
			image.setPixels(c.second.second);
			FreeImageWrapper::saveImage(legendPrefix + "instance_" + c.second.first + ".png", image);
		}
		for (const auto& c : labelColors) {
			ColorImageR8G8B8 image(100, 100);
			image.setPixels(c.second.second);
			FreeImageWrapper::saveImage(legendPrefix + "label_" + c.second.first + ".png", image);
		}
	}
}

ColorImageR32 convertToGrayscale(const ColorImageR8G8B8& color)
{
	ColorImageR32 res(color.getDimensions());
	for (const auto& p : color) {
		float v = (0.299f*p.value.x + 0.587f*p.value.y + 0.114f*p.value.z) / 255.0f;
		res(p.x, p.y) = v;
	}
	return res;
}

void convertToGrayscale(const vec3uc* color, unsigned int width, unsigned int height, ColorImageR32& intensity)
{
	intensity.allocate(width, height);
	const float inv = 1.0f / 255.0f;
	for (unsigned int y = 0; y < height; y++) {
		for (unsigned int x = 0; x < width; x++) {
			const vec3uc& c = color[y * width + x];
			float v = (0.299f*c.x + 0.587f*c.y + 0.114f*c.z) * inv;
			intensity(x, y) = v;
		}
	}
}

void convertToFloat(const unsigned short* depth, unsigned int width, unsigned int height, DepthImage32& depthImage)
{
	depthImage.allocate(width, height);
	const float inv = 1.0f / 255.0f;
	for (unsigned int y = 0; y < height; y++) {
		for (unsigned int x = 0; x < width; x++) {
			const unsigned short d = depth[y*width + x];
			if (d == 0) depthImage(x, y) = -std::numeric_limits<float>::infinity();
			else depthImage(x, y) = (float)d * 0.001f;
		}
	}
}

void process(FilterData& filterData, std::string path, std::string scanNetPath, std::string outputPath, const std::string& name, bool printDebugOutput)
{
	if (!(path.back() == '\\' || path.back() == '/')) path.push_back('/');
	if (!(scanNetPath.back() == '\\' || scanNetPath.back() == '/')) scanNetPath.push_back('/');
	if (!(outputPath.back() == '\\' || outputPath.back() == '/')) outputPath.push_back('/');
	const std::string instancePath = path + "instance/";
	const std::string labelPath = path + "label/";
	if (!util::directoryExists(labelPath) || !util::directoryExists(instancePath))
		throw MLIB_EXCEPTION("instance/label dir does not exist for " + path);
	const std::string outputInstancePath = outputPath + "instance/";
	const std::string outputLabelPath = outputPath + "label/";
	std::cout << path << std::endl;
	SensorData sd(scanNetPath + name + "/" + name + ".sens");
	if (util::directoryExists(outputInstancePath) && util::directoryExists(outputLabelPath)) {
		Directory outInst(outputInstancePath);
		Directory outLabel(outputLabelPath);
		if (outInst.getFiles().size() == sd.m_frames.size() && outLabel.getFiles().size() == sd.m_frames.size()) {
			std::cout << "  ==> skipping, already exists" << std::endl;
			return;
		}
	}
	Aggregation agg; agg.loadFromJSONFile(scanNetPath + name + "/" + name + ".aggregation.json");
	const auto& objecIdsToLabelNames = agg.getObjectIdsToLabels();

	if (!util::directoryExists(outputPath)) util::makeDirectory(outputPath);
	if (!util::directoryExists(outputInstancePath)) util::makeDirectory(outputInstancePath);
	if (!util::directoryExists(outputLabelPath)) util::makeDirectory(outputLabelPath);
	const std::string outDebugPath = "debug/" + name + "/";
	if (printDebugOutput && !util::directoryExists(outDebugPath)) util::makeDirectory(outDebugPath);

	const std::vector<unsigned int> filterWidths = { 320, sd.m_colorWidth };
	const std::vector<unsigned int> filterHeights = { 240, sd.m_colorHeight };
	const std::vector<unsigned int> filterRadii = { 12, 10 };
	const std::vector<float> intensityScales = { 10.0f, 4.0f }; //higher respects edges better but edges are rough and sometimes ends up cutting things that overreach too far
	MLIB_ASSERT(filterWidths.size() == filterHeights.size() && filterWidths.size() == intensityScales.size());
	const unsigned int numFiltIters = (unsigned int)filterWidths.size();

	std::vector<unsigned char> objectIdsToIdx(MAX_NUM_LABELS_PER_SCENE, 255), idxToObjectIds(MAX_NUM_LABELS_PER_SCENE, 255);
	std::vector<unsigned short> objectIdsToLabel(MAX_NUM_LABELS_PER_SCENE, 65535);
	unsigned char idx = 1;
	objectIdsToIdx[0] = 0; idxToObjectIds[0] = 0; objectIdsToLabel[0] = 0;
	for (const auto& a : objecIdsToLabelNames) { //object ids are 0-indexed so add 1
		unsigned short label;
		bool bValid = LabelUtil::get().getIdForLabel(a.second, label);
		if (!bValid) {
			//std::cout << "warning: no label id for " << a.second << std::endl;
			label = 0;
		}
		objectIdsToLabel[a.first + 1] = label;
		objectIdsToIdx[a.first + 1] = idx;
		idxToObjectIds[idx] = a.first + 1;
		idx++;
	}
	filterData.init(objectIdsToIdx, idxToObjectIds, objectIdsToLabel);

	Directory dir(labelPath);
	const auto& files = dir.getFiles(); unsigned int _idx = 0;
	for (const auto& f : files) {
		const unsigned int frameIdx = util::convertTo<unsigned int>(util::removeExtensions(f));
		if (sd.m_frames[frameIdx].getCameraToWorld()[0] == -std::numeric_limits<float>::infinity()) {
			BaseImage<unsigned short> labelImage(sd.m_colorWidth, sd.m_colorHeight); BaseImage<unsigned char> instanceImage(sd.m_colorWidth, sd.m_colorHeight);
			labelImage.setPixels(0); instanceImage.setPixels(0);
			FreeImageWrapper::saveImage(outputInstancePath + f, instanceImage);
			FreeImageWrapper::saveImage(outputLabelPath + f, labelImage);
			continue;
		}
		DepthImage32 depth; ColorImageR32 intensity;
		{
			unsigned short* depthData = sd.decompressDepthAlloc(frameIdx);
			vec3uc* colorData = sd.decompressColorAlloc(frameIdx);
			convertToFloat(depthData, sd.m_depthWidth, sd.m_depthHeight, depth);
			convertToGrayscale(colorData, sd.m_colorWidth, sd.m_colorHeight, intensity); //could also move to gpu
			std::free(depthData); std::free(colorData);
		}
		MLIB_CUDA_SAFE_CALL(cudaMemcpy(filterData.d_depth, depth.getData(), sizeof(float)*depth.getNumPixels(), cudaMemcpyHostToDevice));
		MLIB_CUDA_SAFE_CALL(cudaMemcpy(filterData.d_intensity, intensity.getData(), sizeof(float)*intensity.getNumPixels(), cudaMemcpyHostToDevice));
		bilateralFilterFloatMap(filterData.d_intensityHelper, filterData.d_intensity, 6.0f, 0.1f, intensity.getWidth(), intensity.getHeight());
		bilateralFilterFloatMap(filterData.d_depthHelper, filterData.d_depth, 2.0f, 0.1f, depth.getWidth(), depth.getHeight());

		const std::string instanceFile = instancePath + f;
		const std::string labelFile = labelPath + f;
		BaseImage<unsigned short> labelImage; BaseImage<unsigned char> instanceImage;
		FreeImageWrapper::loadImage(instanceFile, instanceImage);
		FreeImageWrapper::loadImage(labelFile, labelImage);
		MLIB_CUDA_SAFE_CALL(cudaMemcpy(filterData.d_instanceHelper, instanceImage.getData(), sizeof(unsigned char)*instanceImage.getNumPixels(), cudaMemcpyHostToDevice));

		if (printDebugOutput && frameIdx % 100 == 0) {//debug vis
			FreeImageWrapper::saveImage(outDebugPath + std::to_string(frameIdx) + "_depth.png", ColorImageR32G32B32(depth));
			FreeImageWrapper::saveImage(outDebugPath + std::to_string(frameIdx) + "_intensity.png", intensity);
			MLIB_CUDA_SAFE_CALL(cudaMemcpy(intensity.getData(), filterData.d_intensityHelper, sizeof(float)*intensity.getNumPixels(), cudaMemcpyDeviceToHost));
			FreeImageWrapper::saveImage(outDebugPath + std::to_string(frameIdx) + "_intensity-filt.png", intensity);
			visualizeAnnotations(outDebugPath + std::to_string(frameIdx) + "_orig", instanceImage, labelImage);
		}//debug vis

		//t.start();
		unsigned int curDepthWidth = depth.getWidth(), curDepthHeight = depth.getHeight();
		unsigned int curColorWidth = intensity.getWidth(), curColorHeight = intensity.getHeight();
		if (filterWidths.front() != instanceImage.getWidth())
			resampleUCharMap(filterData.d_instance, filterWidths.front(), filterHeights.front(), filterData.d_instanceHelper, instanceImage.getWidth(), instanceImage.getHeight());
		for (unsigned int iter = 0; iter < numFiltIters; iter++) {
			//resample depth
			if (curDepthWidth != filterWidths[iter]) {
				if (filterWidths[iter] == sd.m_depthWidth) {
					if (iter + 1 == numFiltIters)
						std::swap(filterData.d_depth, filterData.d_depthHelper);
					else
						MLIB_CUDA_SAFE_CALL(cudaMemcpy(filterData.d_depth, depth.getData(), sizeof(float)*depth.getNumPixels(), cudaMemcpyHostToDevice));
				}
				else {
					resampleFloatMap(filterData.d_depth, filterWidths[iter], filterHeights[iter], filterData.d_depthHelper, depth.getWidth(), depth.getHeight());
				}
				curDepthWidth = filterWidths[iter];
				curDepthHeight = filterHeights[iter];
			}
			//resample color
			if (curColorWidth != filterWidths[iter]) {
				if (filterWidths[iter] == sd.m_colorWidth) {
					if (iter + 1 == numFiltIters)
						std::swap(filterData.d_intensity, filterData.d_intensityHelper);
					else
						MLIB_CUDA_SAFE_CALL(cudaMemcpy(filterData.d_intensity, intensity.getData(), sizeof(float)*intensity.getNumPixels(), cudaMemcpyHostToDevice));
				}
				else {
					resampleFloatMap(filterData.d_intensity, filterWidths[iter], filterHeights[iter], filterData.d_intensityHelper, intensity.getWidth(), intensity.getHeight());
				}
				curColorWidth = filterWidths[iter];
				curColorHeight = filterHeights[iter];
			}
			filterAnnotations(filterData.d_instanceHelper, filterData.d_instance, filterData.d_depth, filterData.d_intensity,
				filterData.d_instanceToIdx, filterData.d_idxToInstance, filterData.d_vote,
				filterRadii[iter], filterWidths[iter], filterHeights[iter], 5.0f, 0.1f, intensityScales[iter]);

			if (iter + 1 == numFiltIters)
				std::swap(filterData.d_instanceHelper, filterData.d_instance); //result is in instance
			else
				resampleUCharMap(filterData.d_instance, filterWidths[iter + 1], filterHeights[iter + 1], filterData.d_instanceHelper, filterWidths[iter], filterHeights[iter]);
		}
		convertInstanceToLabel(filterData.d_label, filterData.d_instance, filterData.d_instanceToLabel, filterWidths.back(), filterHeights.back());
		MLIB_CUDA_SAFE_CALL(cudaMemcpy(instanceImage.getData(), filterData.d_instance, sizeof(unsigned char)*instanceImage.getNumPixels(), cudaMemcpyDeviceToHost));
		MLIB_CUDA_SAFE_CALL(cudaMemcpy(labelImage.getData(), filterData.d_label, sizeof(unsigned short)*labelImage.getNumPixels(), cudaMemcpyDeviceToHost));

		//save out
		FreeImageWrapper::saveImage(outputInstancePath + f, instanceImage);
		FreeImageWrapper::saveImage(outputLabelPath + f, labelImage);

		if (printDebugOutput && frameIdx % 100 == 0) {//debug vis
			visualizeAnnotations(outDebugPath + std::to_string(frameIdx) + "_filt", instanceImage, labelImage);
			std::cout << "waiting..." << std::endl;
			getchar();
		}
		if (_idx % 10 == 0 || _idx + 1 == files.size())
			std::cout << "\r[ " << _idx << " | " << files.size() << " ]";
		++_idx;
	}
	std::cout << std::endl;
}

void debugVis(const std::string& rawPath, const std::string& filtPath, const std::string& scene, const std::string& scanNetPath, const std::string& outDir)
{
	const unsigned int frameIdx = 400;//0;
	const std::string rawInstanceFile = rawPath + scene + "/instance/" + std::to_string(frameIdx) + ".png";
	const std::string rawLabelFile = rawPath + scene + "/label/" + std::to_string(frameIdx) + ".png";
	const std::string filtInstanceFile = filtPath + scene + "/instance/" + std::to_string(frameIdx) + ".png";
	const std::string filtLabelFile = filtPath + scene + "/label/" + std::to_string(frameIdx) + ".png";

	Aggregation agg; agg.loadFromJSONFile(scanNetPath + scene + "/" + scene + ".aggregation.json");
	SensorData sd(scanNetPath + scene + "/" + scene + ".sens");
	DepthImage32 depth = sd.computeDepthImage(frameIdx);
	ColorImageR8G8B8 color = sd.computeColorImage(frameIdx);

	BaseImage<unsigned char> rawInstance, filtInstance;
	BaseImage<unsigned short> rawLabel, filtLabel;
	FreeImageWrapper::loadImage(rawInstanceFile, rawInstance);
	FreeImageWrapper::loadImage(filtInstanceFile, filtInstance);
	FreeImageWrapper::loadImage(rawLabelFile, rawLabel);
	FreeImageWrapper::loadImage(filtLabelFile, filtLabel);

	visualizeAnnotations(outDir + "raw", rawInstance, rawLabel, true, agg);
	visualizeAnnotations(outDir + "filt", filtInstance, filtLabel);
	FreeImageWrapper::saveImage(outDir + "depth.png", ColorImageR32G32B32(depth));
	FreeImageWrapper::saveImage(outDir + "color.png", color);
}

