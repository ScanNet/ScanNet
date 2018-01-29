
#include "stdafx.h"
#include "Visualizer.h"
#include "GlobalAppState.h"
#include "LabelUtil.h"
#include "omp.h"
#include <sstream>

void Visualizer::init(ApplicationData &app)
{
	std::string scanDir = GlobalAppState::get().s_scanDir;
	scanDir = util::replace(scanDir, '\\', '/');
	if (scanDir.back() != '/') scanDir.push_back('/');
	std::cout << "[ProjectAnnotations] " << scanDir << std::endl;
	const std::string scanName = util::split(scanDir, "/").back();
	const std::string sensFile = scanDir + scanName + ".sens";
	const std::string meshFile = scanDir + scanName + "_vh_clean_2.ply";
	const std::string segsFile = scanDir + scanName + "_vh_clean_2.0.010000.segs.json";
	const std::string aggregationFile = scanDir + scanName + ".aggregation.json";
	const std::string meshHiFile = scanDir + scanName + "_vh_clean.ply";
	const bool bUseHiResMesh = GlobalAppState::get().s_useHiResMesh;
	if (!(util::fileExists(sensFile) && util::fileExists(meshFile) && util::fileExists(segsFile) &&
		util::fileExists(aggregationFile) && (!bUseHiResMesh || util::fileExists(meshHiFile)))) {
		std::cout << "WARNING: no sens/mesh/segs/aggregation file, skipping" << std::endl;
		return;
	}
	LabelUtil::get().init(GlobalAppState::get().s_labelMappingFile);

	m_sensorName = util::removeExtensions(util::splitPath(sensFile).back());
	std::cout << "loading scan info... "; Timer t;
	m_sensorData.loadFromFile(sensFile);
	//load aggregation and compute segmentation ids
	Segmentation segmentation; segmentation.loadFromFile(segsFile);
	Aggregation aggregation; aggregation.loadFromJSONFile(aggregationFile);

	MeshDataf meshData = MeshIOf::loadFromFile(meshFile);
	MeshDataf meshHi; if (bUseHiResMesh) MeshIOf::loadFromFile(meshHiFile, meshHi);
	unsigned int numObjects = computeObjectIdsAndColorsPerVertex(aggregation, segmentation, meshData, meshHi); //puts hi-res with labels into meshdata
	m_mesh.init(app.graphics, TriMeshf(meshData));
	std::cout << "done! (" << t.getElapsedTime() << " s)" << std::endl;

	m_fieldOfView = 2 * 180 / PI * atan(0.5 * m_sensorData.m_colorWidth / m_sensorData.m_calibrationColor.m_intrinsic(0, 0));
	m_camera = Cameraf(m_sensorData.m_frames[0].getCameraToWorld(), m_fieldOfView,
		(float)m_sensorData.m_colorWidth / (float)m_sensorData.m_colorHeight, GlobalAppState::get().s_depthMin, GlobalAppState::get().s_depthMax);

	m_constants.init(app.graphics);

	std::vector<DXGI_FORMAT> formats = {
		DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT
	};
	m_renderTarget.init(app.graphics.castD3D11(), m_sensorData.m_colorWidth, m_sensorData.m_colorHeight, formats, true);

	const std::string outDir = GlobalAppState::get().s_outDir;
	if (!util::directoryExists(outDir)) util::makeDirectory(outDir);
}

void Visualizer::render(ApplicationData &app)
{
	m_timer.frame();

	static unsigned int frame = 0;
	bool validFrame = false;
	if (frame < m_sensorData.m_frames.size()) {
		if (m_sensorData.m_frames[frame].getCameraToWorld()._m00 != -std::numeric_limits<float>::infinity()) validFrame = true;
		m_camera = Cameraf(m_sensorData.m_frames[frame].getCameraToWorld(), m_fieldOfView,
			(float)m_sensorData.m_colorWidth / (float)m_sensorData.m_colorHeight, GlobalAppState::get().s_depthMin, GlobalAppState::get().s_depthMax);
		std::cout << "\r[ " << (frame + 1) << " | " << m_sensorData.m_frames.size() << " ]";
	}
	else {
		std::cout << std::endl << "done" << std::endl;
		exit(0);
	}

	std::string outDir = GlobalAppState::get().s_outDir;
	outDir = (outDir.back() == '/' || outDir.back() == '\\') ? outDir + m_sensorName + "/" : outDir + "/" + m_sensorName + "/";
	if (!util::directoryExists(outDir)) util::makeDirectory(outDir);
	const std::string outInstanceDir = outDir + "instance/"; if (!util::directoryExists(outInstanceDir)) util::makeDirectory(outInstanceDir);
	const std::string outLabelDir = outDir + "label/"; if (!util::directoryExists(outLabelDir)) util::makeDirectory(outLabelDir);

	const bool bFilterUsingOrigDepth = GlobalAppState::get().s_filterUsingOrigialDepthImage;
	const float zNear = GlobalAppState::get().s_depthMin;
	const float zFar = GlobalAppState::get().s_depthMax;
	const float depthDistThresh = GlobalAppState::get().s_depthDistThresh;
	if (validFrame) {
		DepthImage16 origDepthImage = m_sensorData.computeDepthImage(frame);

		mat4f proj = Cameraf::visionToGraphicsProj(m_sensorData.m_colorWidth, m_sensorData.m_colorHeight, m_sensorData.m_calibrationColor.m_intrinsic(0, 0), m_sensorData.m_calibrationColor.m_intrinsic(1, 1), zNear, zFar);
		ConstantBuffer constants;
		constants.worldViewProj = proj * m_camera.getView();

		constants.modelColor = ml::vec4f(1.0f, 1.0f, 1.0f, 1.0f);
		m_constants.updateAndBind(constants, 0);
		app.graphics.castD3D11().getShaderManager().registerShader("shaders/drawAnnotations.hlsl", "drawAnnotations", "vertexShaderMain", "vs_4_0", "pixelShaderMain", "ps_4_0");
		app.graphics.castD3D11().getShaderManager().bindShaders("drawAnnotations");

		m_renderTarget.clear();
		m_renderTarget.bind();
		m_mesh.render();
		m_renderTarget.unbind();

		DepthImage32 depthBuffer;
		ColorImageR32G32B32A32 colorBuffer;
		m_renderTarget.captureColorBuffer(colorBuffer);
		m_renderTarget.captureDepthBuffer(depthBuffer);
		//annotations
		MLIB_ASSERT(colorBuffer.getWidth() == m_sensorData.m_colorWidth && colorBuffer.getHeight() == m_sensorData.m_colorHeight);
		BaseImage<unsigned char> objectInstanceImage(colorBuffer.getWidth(), colorBuffer.getHeight()); // image with object id annotations
		BaseImage<unsigned short> objectLabelImage(colorBuffer.getWidth(), colorBuffer.getHeight());
		for (unsigned int i = 0; i < colorBuffer.getNumPixels(); i++) {
			const vec4f& c = colorBuffer.getData()[i];
			float label = c.w;			float id = c.z;
			label = std::round(label);	id = std::round(id);
			MLIB_ASSERT(label >= 0 && label < 65535 && id >= 0 && id < 255);
			objectInstanceImage.getData()[i] = (unsigned char)id;
			objectLabelImage.getData()[i] = (unsigned short)label;
		}

		//depth
		mat4f projToCamera = m_camera.getProj().getInverse();
		mat4f cameraToWorld = m_camera.getView().getInverse();
		mat4f projToWorld = cameraToWorld * projToCamera;
		mat4f intrinsic = Cameraf::graphicsToVisionProj(m_camera.getProj(), depthBuffer.getWidth(), depthBuffer.getHeight());
#pragma omp parallel for
		for (int y = 0; y < (int)depthBuffer.getHeight(); y++) {
			for (int x = 0; x < (int)depthBuffer.getWidth(); x++) {
				float d = depthBuffer(x, y);
				if (d != 0.0f && d != 1.0f) {
					vec3f posProj = vec3f(app.graphics.castD3D11().pixelToNDC(vec2i(x, y), depthBuffer.getWidth(), depthBuffer.getHeight()), d);
					vec3f posCamera = projToCamera * posProj;
					if (posCamera.z >= zNear && posCamera.z <= zFar) depthBuffer(x, y) = posCamera.z;
					else depthBuffer(x, y) = 0.0f;
				}
				else {
					depthBuffer(x, y) = 0.0f;
				}
			}
		}
		DepthImage16 depth(depthBuffer.getResized(m_sensorData.m_depthWidth, m_sensorData.m_depthHeight), 1000.0f);

		const float scaleDepthWidth = (float)(depth.getWidth() - 1) / (float)(objectLabelImage.getWidth() - 1);
		const float scaleDepthHeight = (float)(depth.getHeight() - 1) / (float)(objectLabelImage.getHeight() - 1);
#pragma omp parallel for
		for (int y = 0; y < (int)objectLabelImage.getHeight(); y++) {
			for (int x = 0; x < (int)objectLabelImage.getWidth(); x++) {
				unsigned short v = objectLabelImage(x, y);
				if (v != 0) {
					const unsigned int dx = (unsigned int)std::round(scaleDepthWidth * x);
					const unsigned int dy = (unsigned int)std::round(scaleDepthHeight * y);
					const unsigned short drndr = depth(dx, dy);
					const unsigned short dorig = origDepthImage(dx, dy);
					if ((bFilterUsingOrigDepth && dorig == 0) || (drndr != 0 && dorig != 0 && std::fabs((drndr - dorig) * 0.001f) > depthDistThresh + 0.01f * dorig)) {
						objectLabelImage(x, y) = 0;
						objectInstanceImage(x, y) = 0;
					}
				}
			} //x
		} //y

		int radius = 2;
#pragma omp parallel for
		for (int y = 0; y < (int)objectLabelImage.getHeight(); y++) {
			for (int x = 0; x < (int)objectLabelImage.getWidth(); x++) {
				unsigned short v = objectLabelImage(x, y);
				if (v != 0) {
					unsigned int count = 0, total = 0;
					for (int yy = y - radius; yy <= y + radius; yy++) {
						for (int xx = x - radius; xx <= x + radius; xx++) {
							if (xx >= 0 && xx < (int)objectLabelImage.getWidth() && yy >= 0 && yy < (int)objectLabelImage.getHeight()) {
								total++;
								if (objectLabelImage(xx, yy) == v) count++;
							}
						}
					}
					if ((float)count / (float)total < 0.2f) {
						objectLabelImage(x, y) = 0;
						objectInstanceImage(x, y) = 0;
					}
				}
			} //x
		} //y

		FreeImageWrapper::saveImage(outInstanceDir + std::to_string(frame) + ".png", objectInstanceImage);
		FreeImageWrapper::saveImage(outLabelDir + std::to_string(frame) + ".png", objectLabelImage);

		if (GlobalAppState::get().s_outputDebugImages && frame % 100 == 0) {

			static std::unordered_map<unsigned short, vec3uc> colors;
			for (const auto& p : objectLabelImage) {
				if (p.value != 0 && colors.find(p.value) == colors.end()) {
					RGBColor c = RGBColor::randomColor();
					colors[p.value] = vec3uc(c.x, c.y, c.z);
				}
			}
			for (const auto& p : objectInstanceImage) {
				if (p.value != 0 && colors.find(p.value) == colors.end()) {
					RGBColor c = RGBColor::randomColor();
					colors[p.value] = vec3uc(c.x, c.y, c.z);
				}
			}

			//debug print out colored annotation image
			ColorImageR8G8B8 colorImageInstance(colorBuffer.getWidth(), colorBuffer.getHeight());
			ColorImageR8G8B8 colorImageLabel(colorBuffer.getWidth(), colorBuffer.getHeight());
			for (const auto& p : objectLabelImage) {
				if (p.value != 0) {
					const auto it = colors.find(p.value);
					MLIB_ASSERT(it != colors.end());
					colorImageLabel(p.x, p.y) = it->second;
				}
				else colorImageLabel(p.x, p.y) = vec3uc(0, 0, 0);
			}
			for (const auto& p : objectInstanceImage) {
				if (p.value != 0) {
					const auto it = colors.find(p.value);
					MLIB_ASSERT(it != colors.end());
					colorImageInstance(p.x, p.y) = it->second;
				}
				else colorImageInstance(p.x, p.y) = vec3uc(0, 0, 0);
			}
			FreeImageWrapper::saveImage(outDir + std::to_string(frame) + "_color-label.png", colorImageLabel);
			FreeImageWrapper::saveImage(outDir + std::to_string(frame) + "_color-instance.png", colorImageInstance);
			//std::cout << "waiting..." << std::endl; getchar();
		}
	}
	else {
		BaseImage<unsigned char> objectInstanceImage(m_sensorData.m_colorWidth, m_sensorData.m_colorHeight, (unsigned char)0); //empty image, no valid transform
		BaseImage<unsigned short> objectLabelImage(m_sensorData.m_colorWidth, m_sensorData.m_colorHeight, (unsigned short)0); //empty image, no valid transform
		FreeImageWrapper::saveImage(outInstanceDir + std::to_string(frame) + ".png", objectInstanceImage);
		FreeImageWrapper::saveImage(outLabelDir + std::to_string(frame) + ".png", objectLabelImage);
	}
	frame += GlobalAppState::get().s_frameSkip;
}

void Visualizer::resize(ApplicationData &app)
{
	m_camera.updateAspectRatio((float)app.window.getWidth() / app.window.getHeight());
}

void Visualizer::keyDown(ApplicationData &app, UINT key)
{
}

void Visualizer::keyPressed(ApplicationData &app, UINT key)
{
}

void Visualizer::mouseDown(ApplicationData &app, MouseButtonType button)
{
}

void Visualizer::mouseWheel(ApplicationData &app, int wheelDelta)
{
}

void Visualizer::mouseMove(ApplicationData &app)
{
}

unsigned int Visualizer::computeObjectIdsAndColorsPerVertex(const Aggregation& aggregation, const Segmentation& segmentation,
	MeshDataf& meshData, const MeshDataf& meshHi)
{
	const bool bUseHi = !meshHi.isEmpty();
	std::vector<vec4f> colorsPerVertex(meshData.m_Vertices.size(), vec4f::origin);

	const auto& aggregatedSegments = aggregation.getAggregatedSegments();
	const auto& objectIdsToLabels = aggregation.getObjectIdsToLabels();
	//generate some random colors
	std::unordered_map<unsigned short, vec4f> objectColors; std::unordered_map<unsigned int, unsigned short> objectIdsToLabelIds;
	for (unsigned int i = 0; i < aggregatedSegments.size(); i++) {
		const unsigned int objectId = i;
		const auto itl = objectIdsToLabels.find(objectId);
		MLIB_ASSERT(itl != objectIdsToLabels.end());
		unsigned short labelId;
		if (LabelUtil::get().getIdForLabel(itl->second, labelId)) {
			objectIdsToLabelIds[objectId] = labelId;
			auto itc = objectColors.find(labelId);
			if (itc == objectColors.end()) {
				RGBColor c = RGBColor::randomColor();
				objectColors[labelId] = vec4f(c.x / 255.0f, c.y / 255.0f, objectId + 1, labelId); //objectid -> instance, labelid-> label
			}
		}
	}
	//assign object ids and colors
	std::unordered_map< unsigned int, std::vector<unsigned int> > verticesPerSegment = segmentation.getSegmentIdToVertIdMap();
	for (unsigned int i = 0; i < aggregatedSegments.size(); i++) {
		const unsigned int objectId = i;
		const auto itl = objectIdsToLabelIds.find(objectId);
		if (itl == objectIdsToLabelIds.end()) continue;
		const vec4f& color = objectColors[itl->second];
		for (unsigned int seg : aggregatedSegments[i]) {
			const std::vector<unsigned int>& vertIds = verticesPerSegment[seg];
			for (unsigned int v : vertIds) {
				colorsPerVertex[v] = color;
			}
		}
	}
	meshData.m_Colors = colorsPerVertex;
	if (bUseHi) {
		MeshDataf propagated = meshHi;
		if (!meshData.hasNormals()) meshData.computeVertexNormals();
		if (!propagated.hasNormals()) propagated.computeVertexNormals();
		propagateAnnotations(meshData, propagated);
		meshData = propagated;
	}
	return (unsigned int)objectColors.size();
}

void Visualizer::propagateAnnotations(const MeshDataf& meshSrc, MeshDataf& meshDst)
{
	MLIB_ASSERT(meshSrc.hasNormals() && meshDst.hasNormals());
	const float normalThresh = GlobalAppState::get().s_propagateNormalThresh;

	const bbox3f bbox = meshSrc.computeBoundingBox();

	//nearest neighbor search for vertices
	std::vector<vec3f> searchVerts;
	std::vector<unsigned int> searchIndices;
	for (unsigned int i = 0; i < meshSrc.m_Vertices.size(); i++) {
		if (meshSrc.m_Colors[i].w > 0) {
			searchVerts.push_back(meshSrc.m_Vertices[i]);
			searchIndices.push_back(i);
		}
	}
	std::vector<const float*> srcVerts(searchVerts.size());
	for (unsigned int i = 0; i < searchVerts.size(); i++) {
		srcVerts[i] = (const float*)&searchVerts[i];
	}
	const unsigned int maxK = 3;
	const float eps = 0.01f;
	int numThreads = omp_get_max_threads();
	std::vector<NearestNeighborSearchFLANNf*> nn(numThreads);
	for (unsigned int i = 0; i < nn.size(); i++) {
		nn[i] = new NearestNeighborSearchFLANNf(100, 12);
		nn[i]->init(srcVerts, 3, maxK);
	}
	const float maxThresh = std::max(bbox.getMaxExtent() * 0.01f, 0.05f);

	if (!meshDst.hasColors()) meshDst.m_Colors.resize(meshDst.m_Vertices.size());
#pragma omp parallel for 
	for (int i = 0; i < (int)meshDst.m_Vertices.size(); i++) {
		int thread = omp_get_thread_num();
		std::vector<unsigned int> nearestIndices;
		nn[thread]->kNearest((const float*)&meshDst.m_Vertices[i], maxK, eps, nearestIndices);
		if (!nearestIndices.empty()) {
			std::vector<float> dists = nn[thread]->getDistances(maxK);
			unsigned int bestIdx = (unsigned int)-1;
			const vec4f& val = meshSrc.m_Colors[searchIndices[nearestIndices[0]]]; //object id
			bool allSame = true;
			for (unsigned int k = 0; k < maxK; k++) {
				if (dists[k] < maxThresh) {
					if (std::acos(math::clamp(meshSrc.m_Normals[searchIndices[nearestIndices[k]]] | meshDst.m_Normals[i], -1.0f, 1.0f)) < normalThresh) { //make sure similar normals
						bestIdx = k;
						break;
					}
					if (meshSrc.m_Colors[searchIndices[nearestIndices[k]]].z != val.z) allSame = false;
				}
				else allSame = false;
			}
			if (bestIdx != (unsigned int)-1) {
				const vec3f& nearest = searchVerts[nearestIndices[bestIdx]];
				meshDst.m_Colors[i] = meshSrc.m_Colors[searchIndices[nearestIndices[bestIdx]]];
			}
			else if (allSame) {
				meshDst.m_Colors[i] = val;
			}
			else {
				meshDst.m_Colors[i] = vec4f(0.0f);
			}
		}
		else {
			meshDst.m_Colors[i] = vec4f(0.0f);
		}
	}
	for (unsigned int i = 0; i < nn.size(); i++)
		SAFE_DELETE(nn[i]);
}


