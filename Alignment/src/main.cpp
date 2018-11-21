
#include "stdafx.h"

#include "main.h"
#include "alignment.h"

void alignScan(const std::string& sceneFolder, bool forceRealign = false) 
{
	std::cout << "aligning: " << sceneFolder << std::endl;
	Alignment::alignScan(sceneFolder, forceRealign);
}

void alignScanFromAlnFile(const std::string& sceneFolder) 
{
	std::cout << "aligning: " << sceneFolder << std::endl;
	Alignment::alignScanFromAlnFile(sceneFolder);
}



void alignDirectory(const std::string& path, bool forceRealign = false) {
	Directory dir(path);
	std::vector<std::string> scenes = dir.getDirectories();
	std::sort(scenes.begin(), scenes.end());
	for (size_t i = 0; i < scenes.size(); i++) {
		const std::string& scene = scenes[i];
		const std::string sceneFolder = path + scene;
		alignScan(sceneFolder);
		//break;
		//if (i >= 20) break;
	}
}

int main(int argc, char* argv[])
{
	try {
		if (argc == 2) { //converts a specific scan given by the command line argument
			std::string stagingFolder(argv[1]);
			alignScan(stagingFolder);
		}
		else {
			throw MLIB_EXCEPTION("requires the path as a command line argument");
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

	//std::cout << "<press key to end program>" << std::endl;
	//getchar();

	return 0;
}


