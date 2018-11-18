
#pragma  once

#include "stdafx.h"

#ifndef VAR_NAME
#define VAR_NAME(x) #x
#endif

struct MetaData {

	MetaData(const std::string& filename) {
		loadFromFile(filename);
	}

	void loadFromFile(const std::string& filename) {
		ml::ParameterFile parameterFile(filename);
		readParameter(parameterFile, std::string(VAR_NAME(numColorFrames)), numColorFrames);
		readParameter(parameterFile, std::string(VAR_NAME(numDepthFrames)), numDepthFrames);
		readParameter(parameterFile, std::string(VAR_NAME(numIMUmeasurements)), numIMUmeasurements);
		readParameter(parameterFile, std::string(VAR_NAME(colorWidth)), colorWidth);
		readParameter(parameterFile, std::string(VAR_NAME(colorHeight)), colorHeight);
		readParameter(parameterFile, std::string(VAR_NAME(depthWidth)), depthWidth);
		readParameter(parameterFile, std::string(VAR_NAME(depthHeight)), depthHeight);

		{
			float fx_color, fy_color, mx_color, my_color;
			readParameter(parameterFile, std::string(VAR_NAME(fx_color)), fx_color);
			readParameter(parameterFile, std::string(VAR_NAME(fy_color)), fy_color);
			readParameter(parameterFile, std::string(VAR_NAME(mx_color)), mx_color);
			readParameter(parameterFile, std::string(VAR_NAME(my_color)), my_color);
			colorCalibration = ml::SensorData::CalibrationData(
				ml::mat4f(
				fx_color, 0.0f, mx_color, 0.0f,
				0.0f, fy_color, my_color, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
				),
				ml::mat4f::identity());
		}
		{
			float fx_depth, fy_depth, mx_depth, my_depth;
			readParameter(parameterFile, std::string(VAR_NAME(fx_depth)), fx_depth);
			readParameter(parameterFile, std::string(VAR_NAME(fy_depth)), fy_depth);
			readParameter(parameterFile, std::string(VAR_NAME(mx_depth)), mx_depth);
			readParameter(parameterFile, std::string(VAR_NAME(my_depth)), my_depth);

			ml::mat4f depthToColorExtrinsics = ml::mat4f::identity();
			if (readParameter(parameterFile, std::string(VAR_NAME(colorToDepthExtrinsics)), depthToColorExtrinsics)) depthToColorExtrinsics.invert();
			depthCalibration = ml::SensorData::CalibrationData(
				ml::mat4f(
				fx_depth, 0.0f, mx_depth, 0.0f,
				0.0f, fy_depth, my_depth, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
				),
				depthToColorExtrinsics);
		}
	}

	template <class T>
	bool readParameter(const ml::ParameterFile& pf, const std::string& varName, T& var) {
		if (!pf.readParameter(varName, var)) { MLIB_WARNING(std::string(varName).append(" ").append("uninitialized")); return false; }
		return true;
	}

	unsigned int numColorFrames;
	unsigned int numDepthFrames;
	unsigned int numIMUmeasurements;
	unsigned int colorWidth;
	unsigned int colorHeight;
	unsigned int depthWidth;
	unsigned int depthHeight;
	ml::SensorData::CalibrationData colorCalibration;
	ml::SensorData::CalibrationData depthCalibration;
};