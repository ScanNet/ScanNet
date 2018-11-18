// sensorFile.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iomanip>

#include "metaData.h"


//converts from seconds to microseconds
UINT64 timeToUINT64(double d) {
	return (UINT64)(d*1000.0*1000.0);
}


void convertToSens(const std::string& baseFilename, ml::SensorData& sens)
{
	sens.free();

	const std::string srcFileMeta = ml::util::removeExtensions(baseFilename) + ".txt";
	const std::string srcFileDepth = ml::util::removeExtensions(baseFilename) + ".depth";
	const std::string srcFileColor = ml::util::removeExtensions(baseFilename) + ".h264";
	const std::string srcFileIMU = ml::util::removeExtensions(baseFilename) + ".imu";


	if (!ml::util::fileExists(srcFileMeta)) throw MLIB_EXCEPTION("file not found " + srcFileMeta);
	if (!ml::util::fileExists(srcFileDepth)) throw MLIB_EXCEPTION("file not found " + srcFileDepth);
	if (!ml::util::fileExists(srcFileColor)) throw MLIB_EXCEPTION("file not found " + srcFileColor);
	if (!ml::util::fileExists(srcFileIMU)) throw MLIB_EXCEPTION("file not found " + srcFileIMU);
	
	MetaData meta(srcFileMeta);
	sens.initDefault(
		meta.colorWidth, meta.colorHeight,
		meta.depthWidth, meta.depthHeight,
		meta.colorCalibration,
		meta.depthCalibration,
		ml::SensorData::COMPRESSION_TYPE_COLOR::TYPE_JPEG,
		ml::SensorData::COMPRESSION_TYPE_DEPTH::TYPE_ZLIB_USHORT,
		1000.0f,
		ml::SensorData::getName().StructureSensor
		);

	std::string execPath = ml::util::getExecutablePath();
	const std::string tmpDir = execPath + "/tmp1337/";
	ml::util::deleteDirectory(tmpDir);	// in case we have a leftover directory

	const std::string baseNameColor = "frame-";
	std::string ffmpegPath = execPath + "../../ffmpeg/ffmpeg.exe";
	if (!ml::util::fileExists(ffmpegPath)) {
		ffmpegPath = execPath + "./ffmpeg/ffmpeg.exe";
	}

	if (!ml::util::fileExists(ffmpegPath)) throw MLIB_EXCEPTION("could not find ffmpeg.exe in path: " + ffmpegPath);
	std::string command =  ffmpegPath + " -i " + srcFileColor + " " + tmpDir + baseNameColor + "%6d.color.png";
	std::cout << "running: " << command << std::endl;
	ml::util::makeDirectory(tmpDir);	
	command = command + " > nul 2>&1";	//pipe output to dev/null
	int res = system(command.c_str());


	std::ifstream inDepth(srcFileDepth, std::ios::binary);

	ml::Directory dir(tmpDir);
	const unsigned int numColorFrames = (unsigned int)dir.getFiles().size();
	unsigned int numFrames = std::min(meta.numDepthFrames, meta.numColorFrames);
	numFrames = std::min(numFrames, numColorFrames);
	if (numFrames != meta.numDepthFrames || numFrames != meta.numColorFrames || numFrames != numColorFrames) {
		MLIB_WARNING("frame counts are different:numColorImages(" + std::to_string(numColorFrames) + ") meta.numDepthImages(" + std::to_string(meta.numDepthFrames) + ") meta.numColorImages(" + std::to_string(meta.numColorFrames) + ")");
	}

	unsigned int frame = 0;
	ml::SensorData::StringCounter scColor(tmpDir + "/" + baseNameColor, "color.png", 6);	scColor.getNext();

	while (frame < numFrames) {
		assert(inDepth.good());

		//ml::Timer t;
		uint32_t byteSize;
		inDepth.read((char*)&byteSize, sizeof(uint32_t));
		char* depthCompressed = new char[byteSize];
		inDepth.read((char*)depthCompressed, byteSize);

		const unsigned int depthWidth = meta.depthWidth;
		const unsigned int depthHeight = meta.depthHeight;

		unsigned short* depth = (unsigned short*)std::malloc(depthWidth*depthHeight * 2);

		uplinksimple::decode((unsigned char*)depthCompressed, (unsigned int)byteSize, depthWidth*depthHeight, depth);
		uplinksimple::shift2depth(depth, depthWidth*depthHeight);

		//check for invalid values
		for (unsigned int i = 0; i < depthWidth*depthHeight; i++) {
			if (depth[i] >= uplinksimple::shift2depth(0xffff)) {
				depth[i] = 0;
			}
		} 

		SAFE_DELETE(depthCompressed);

		std::string colorFile = scColor.getNext();
		ml::ColorImageR8G8B8 cImage;
		ml::FreeImageWrapper::loadImage(colorFile, cImage);
		
		sens.addFrame(cImage.getData(), depth);

		std::free(depth);
		std::cout << "\rframe " << frame << ": read " << byteSize << " [bytes] ";
		frame++;

		//std::cout << t.getElapsedTimeMS() << " [ms]" << std::endl << std::endl;
	}

	//read the rest of the depth frames
	for (; frame < meta.numDepthFrames; frame++) {
		assert(inDepth.good());
		uint32_t byteSize;
		inDepth.read((char*)&byteSize, sizeof(uint32_t));
		char* depthCompressed = new char[byteSize];
		inDepth.read((char*)depthCompressed, byteSize);
		SAFE_DELETE_ARRAY(depthCompressed);
	}


	for (unsigned int i = 0; i < meta.numDepthFrames; i++) {
		//TODO read time stamps here
		double timeStampDouble;
		inDepth.read((char*)&timeStampDouble, sizeof(double));
		UINT64 timeStamp = timeToUINT64(timeStampDouble);
		 
		if (i < (unsigned int)sens.m_frames.size()) {
			sens.m_frames[i].setTimeStampColor(timeStamp);
			sens.m_frames[i].setTimeStampDepth(timeStamp);
		}

		//std::cout << timeStamp << std::endl;
		//std::cout << std::setprecision(20) << timeStampDouble << std::endl;
	}


	std::ifstream inIMU(srcFileIMU, std::ios::binary);
	for (unsigned int i = 0; i < meta.numIMUmeasurements; i++) {
		ml::SensorData::IMUFrame f;	
		double timeStamp = 0.0;
		inIMU.read((char*)&timeStamp, sizeof(double));

		inIMU.read((char*)&f.rotationRate, sizeof(ml::vec3d));
		inIMU.read((char*)&f.acceleration, sizeof(ml::vec3d));
		inIMU.read((char*)&f.magneticField, sizeof(ml::vec3d));
		inIMU.read((char*)&f.attitude, sizeof(ml::vec3d));
		inIMU.read((char*)&f.gravity, sizeof(ml::vec3d));
		f.timeStamp = timeToUINT64(timeStamp);

		if (f.timeStamp == 0) {
			std::cout << "invalid IMUFrame -> skipping" << std::endl;
			continue;
		}
		sens.addIMUFrame(f);
	}
	inIMU.close();

	std::cout << std::endl; 

	//for (size_t i = 0; i < sens.m_frames.size(); i++) {
	//	const ml::SensorData::IMUFrame& f = sens.findClosestIMUFrame(i);
	//	std::cout << "frame time: " << sens.m_frames[i].getTimeStampColor() << "\t imu time: " << f.timeStamp << std::endl;
	//}
	 
	for (size_t i = 0; i < sens.m_frames.size(); i++) {
		const ml::mat4f& t = sens.m_frames[i].getCameraToWorld();
		const ml::mat4f id = ml::mat4f::identity();
		for (unsigned int i = 0; i < 16; i++) {
			if (t[i] != id[i]) throw MLIB_EXCEPTION("matrix needs to be the identity");
		}
		
		//std::cout << sens.m_frames[i].getCameraToWorld() << std::endl;
	}

	ml::util::deleteDirectory(tmpDir);
}

void processStagingFolder(std::string stagingFolder, const std::string& outSensFilename, bool forceOverwrite = false)
{
	stagingFolder = ml::util::replace(stagingFolder, "\\", "/");	//we assume forward slashes

	ml::Directory stagingFolderDir(stagingFolder);
	
	const std::vector<std::string> tmp = ml::util::split(stagingFolder, "/");
	const std::string base = tmp.back();
	const std::string baseFile = stagingFolder + "/" + base;

	std::cout << "converting: " << baseFile << std::endl;

	//const std::string sensFile = baseFile + ".sens";
	if (ml::util::fileExists(outSensFilename)) {
		std::cout << "sensFile already available: " << outSensFilename << "\n\t -> skipping folder" << std::endl;
		return;
	}

	ml::SensorData sd;
	convertToSens(baseFile, sd);
	sd.saveToFile(outSensFilename);
	std::cout << sd << std::endl;

}



int main(int argc, char* argv[])
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	//_CrtSetBreakAlloc(7545);
	try { 

		if (argc == 3) { //converts a specific scan given by the command line argument
			std::string stagingFolder(argv[1]);
			std::string outSensFilename(argv[2]);
			std::cout << stagingFolder << std::endl;
			std::cout << outSensFilename << std::endl;
			processStagingFolder(stagingFolder, outSensFilename);
		}
		else {
			throw MLIB_EXCEPTION("requires the path and output file as a command line arguments");
		}
	}
	catch (const std::exception& e)
	{
		MessageBoxA(NULL, e.what(), "Exception caught", MB_ICONERROR);
		exit(EXIT_FAILURE);
	}
	catch (...)
	{
		MessageBoxA(NULL, "UNKNOWN EXCEPTION", "Exception caught", MB_ICONERROR);
		exit(EXIT_FAILURE);
	}
	//std::cout << "<press key to continue>" << std::endl;
	//getchar();
	return 0;
}

