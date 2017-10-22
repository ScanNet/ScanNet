

#include "stdafx.h"
#include "Visualizer.h"
#include "GlobalAppState.h"

int _tmain(int argc, _TCHAR* argv[])
{
	Visualizer callback;

	std::string fileNameDescGlobalApp = "zParametersScan.txt";
	std::string scanDir = "";
	if (argc == 3) { //overwrite scanDir with command line args
		std::wstring argParam = std::wstring(argv[1]);
		std::wstring argScanDir = std::wstring(argv[2]);
		fileNameDescGlobalApp =std::string(argParam.begin(), argParam.end());
		scanDir = std::string(argScanDir.begin(), argScanDir.end());
	}
	std::cout << VAR_NAME(fileNameDescGlobalApp) << " = " << fileNameDescGlobalApp << std::endl;
	ParameterFile parameterFileGlobalApp(fileNameDescGlobalApp);
	GlobalAppState::get().readMembers(parameterFileGlobalApp);
	if (!scanDir.empty()) GlobalAppState::get().s_scanDir = scanDir;
	GlobalAppState::get().print();

	//get image size
	unsigned int colorWidth = 0, colorHeight = 0;
	scanDir = GlobalAppState::get().s_scanDir;
	scanDir = util::replace(scanDir, '\\', '/');
	if (scanDir.back() != '/') scanDir.push_back('/');
	const std::string scanName = util::split(scanDir, "/").back();
	const std::string metaFile = scanDir + scanName + ".txt";
	if (util::fileExists(metaFile)) {
		ParameterFile pf(metaFile);
		if (!pf.readParameter<unsigned int>("colorWidth", colorWidth)) { 
			std::cerr << "ERROR: failed to read \"colorWidth\" param from " << metaFile << std::endl;
			return -1; 
		};
		if (!pf.readParameter<unsigned int>("colorHeight", colorHeight)) {
			std::cerr << "ERROR: failed to read \"colorHeight\" param from " << metaFile << std::endl;
			return -1;
		};
	}
	else { 
		std::cout << "ERROR: meta-file (" << metaFile << ") does not exist! " << std::endl;
		return -1;
	}

	ApplicationWin32 app(NULL, colorWidth, colorHeight, "Project Annotations", GraphicsDeviceTypeD3D11, callback);
	app.messageLoop();

	return 0;
}
