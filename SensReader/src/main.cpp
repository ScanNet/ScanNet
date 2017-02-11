

#include "sensorData.h"

//THIS IS A DEMO FUNCTION: HOW TO DECODE .SENS FILES: CHECK IT OUT! (doesn't do anything real though)
void processFrame(const ml::SensorData& sd, size_t frameIdx) {
	
	//de-compress color and depth values
	ml::vec3uc* colorData = sd.decompressColorAlloc(frameIdx);
	unsigned short* depthData = sd.decompressDepthAlloc(frameIdx);

	//dimensions of a color/depth frame
	sd.m_colorWidth;
	sd.m_colorHeight;
	sd.m_depthWidth;
	sd.m_depthHeight;
	
	for (unsigned int i = 0; i < sd.m_depthWidth * sd.m_depthHeight; i++) {
		//convert depth values to m:
		float depth_in_meters = sd.m_depthShift * depthData[i];
	}

	std::free(colorData);
	std::free(depthData);
}


int main(int argc, char* argv[])
{
#ifdef WIN32
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
#endif
	try {
		//non-cached read
		std::string filename = "scene0001_00.sens";
		std::string outDir = "./out/";
		if (argc >= 2) filename = std::string(argv[1]);
		else {
			std::cout << "run ./sens <sensfilename>.sens";
			std::cout << "type in filename manually: ";
			std::cin >> filename;
		}
		if (argc >= 3) outDir = std::string(argv[2]);
		
		std::cout << "filename =\t" << filename << std::endl;
		std::cout << "outDir =\t" << outDir << std::endl;
			
		std::cout << "loading from file... ";
		ml::SensorData sd(filename);
		std::cout << "done!" << std::endl;

		std::cout << sd << std::endl;

		// color frames are written out as 'jpgs' and depth frames as binary dumps: <width : uint32, height : uint32, unsigned short* data>
		sd.saveToImages(outDir);

		//THIS SHOWS HOW DE-COMPRESSION WORKS
		//for (size_t i = 0; i < sd.m_frames.size(); i++) {
		{
			size_t i = 0;
			//std::cout << "\r[ processing frame " << std::to_string(i) << " of " << std::to_string(sd.m_frames.size()) << " ]";
			processFrame(sd, i);
		}


		std::cout << std::endl;
	}
	catch (const std::exception& e)
	{
#ifdef WIN32
		MessageBoxA(NULL, e.what(), "Exception caught", MB_ICONERROR);
#else
		std::cout << "Exception caught! " << e.what() << std::endl;
#endif
		exit(EXIT_FAILURE);
	}
	catch (...)
	{
#ifdef WIN32
		MessageBoxA(NULL, "UNKNOWN EXCEPTION", "Exception caught", MB_ICONERROR);
#else
		std::cout << "Exception caught! (unknown)";
#endif
		exit(EXIT_FAILURE);
	}
	
	std::cout << "All done :)" << std::endl;

#ifdef WIN32
	std::cout << "<press key to continue>" << std::endl;
	getchar();
#endif
	return 0;
}

