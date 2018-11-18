
#include "stdafx.h"

#include "main.h"
#include "calibration.h"
#include "aligner.h"

std::string getCalibrationNameFromMap(const std::string& deviceCalibrationMapCsv, std::string scanDirectory) {
	if (!util::directoryExists(scanDirectory)) throw MLIB_EXCEPTION(scanDirectory + " does not exist!");
	scanDirectory = util::replace(scanDirectory, '\\', '/');
	if (scanDirectory.back() == '/') scanDirectory.pop_back();
	const std::string scanMetaFile = scanDirectory + "/" + util::split(scanDirectory, '/').back() + ".txt";
	if (!util::fileExists(scanMetaFile)) return "";
	ParameterFile pf(scanMetaFile); std::string deviceId;
	if (!pf.readParameter<std::string>("deviceId", deviceId)) throw MLIB_EXCEPTION("no device id in meta file: " + scanMetaFile);

	std::unordered_map<std::string, std::string> deviceIdToCalibrationName;
	const std::string deviceIdHeader = "id", calibNameHeader = "calibration_name";
	unsigned int deviceIdIndex = (unsigned int)-1, calibNameIndex = (unsigned int)-1;
	std::ifstream s(deviceCalibrationMapCsv);
	//read header
	std::string line;
	if (!std::getline(s, line)) throw MLIB_EXCEPTION("failed to read device calibration map csv: " + deviceCalibrationMapCsv);
	const auto headers = util::split(line, ',');
	for (unsigned int i = 0; i < headers.size(); i++) {
		if (headers[i] == deviceIdHeader) deviceIdIndex = i;
		if (headers[i] == calibNameHeader) calibNameIndex = i;
	}
	if (deviceIdIndex == (unsigned int)-1 || calibNameIndex == (unsigned int)-1) throw MLIB_EXCEPTION("unable to find device id/calibration name in device calibration map cs file: " + deviceCalibrationMapCsv);
	//find our device id
	while (std::getline(s, line)) {
		const auto elements = util::split(line, ',', true); //include empty strings
		if (elements[deviceIdIndex] == deviceId) {
			std::cout << "\tdevice id: " << deviceId << ", calibration name: " << elements[calibNameIndex] << std::endl;
			return elements[calibNameIndex];
		}
	}
	s.close();

	return ""; //nothing found
}


int main(int argc, char* argv[])
{
	try {
		if (argc == 5) { //converts a specific scan given by the command line arguments ( input_sens_file, output_sens_file, device_calibration_map, device_calibration_directory )
			const std::string inputSensFilename(argv[1]);
			const std::string outputSensFilename(argv[2]);
			const std::string deviceCalibrationMapFile(argv[3]);
			std::string deviceCalibrationDir(argv[4]); 
			if (!(deviceCalibrationDir.back() == '/' || deviceCalibrationDir.back() == '\\')) deviceCalibrationDir.push_back('/');

			const std::string calibrationName = getCalibrationNameFromMap(deviceCalibrationMapFile, util::directoryFromPath(inputSensFilename));
			if (!calibrationName.empty()) {
				Calibration cb;
				cb.calibrateScan(inputSensFilename, outputSensFilename,
					deviceCalibrationDir + calibrationName + ".txt", deviceCalibrationDir + calibrationName + ".lut");
			}
			else std::cout << "no calibration name found" << std::endl;
		}
		else {
			throw MLIB_EXCEPTION("requires the input sens filepath, output sens filepath, parameter file, and input undistortion table as a command line arguments");
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

	return 0;
}


