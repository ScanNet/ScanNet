#pragma once

#include "stdafx.h"
#include "grid3d.h"

#include "aligner.h"

#include "omp.h"

class Calib {
public:
	Calib(const std::string& str) {
		reset();
		readFromFile(str);
	}
	~Calib() {
	}

	void readFromFile(const std::string& str) {
		ParameterFile	parameterFile(str);

		reset();

		parameterFile.readParameter("colorWidth", color_width);
		parameterFile.readParameter("colorHeight", color_height);
		parameterFile.readParameter("fx_color", color_intrinsic(0, 0));
		parameterFile.readParameter("fy_color", color_intrinsic(1, 1));
		parameterFile.readParameter("mx_color", color_intrinsic(0, 2));
		parameterFile.readParameter("my_color", color_intrinsic(1, 2));
		parameterFile.readParameter("k1_color", color_dist_coeff[0]);
		parameterFile.readParameter("k2_color", color_dist_coeff[1]);
		parameterFile.readParameter("k3_color", color_dist_coeff[2]);
		parameterFile.readParameter("k4_color", color_dist_coeff[3]);
		parameterFile.readParameter("k5_color", color_dist_coeff[4]);

		parameterFile.readParameter("depthWidth", depth_width);
		parameterFile.readParameter("depthHeight", depth_height);
		parameterFile.readParameter("fx_depth", depth_intrinsic(0, 0));
		parameterFile.readParameter("fy_depth", depth_intrinsic(1, 1));
		parameterFile.readParameter("mx_depth", depth_intrinsic(0, 2));
		parameterFile.readParameter("my_depth", depth_intrinsic(1, 2));
		parameterFile.readParameter("k1_depth", depth_dist_coeff[0]);
		parameterFile.readParameter("k2_depth", depth_dist_coeff[1]);
		parameterFile.readParameter("k3_depth", depth_dist_coeff[2]);
		parameterFile.readParameter("k4_depth", depth_dist_coeff[3]);
		parameterFile.readParameter("k5_depth", depth_dist_coeff[4]);

		parameterFile.readParameter("depthToColorExtrinsics", depth_extrinsic);
	}

	unsigned int color_width, color_height;
	mat4f color_extrinsic;
	mat4f color_intrinsic;
	float color_dist_coeff[5];

	unsigned int depth_width, depth_height;
	mat4f depth_intrinsic;
	mat4f depth_extrinsic;	//depth-to-color map;
	float depth_dist_coeff[5];

private:
	void reset()
	{
		color_extrinsic.setIdentity();
		color_intrinsic.setIdentity();
		memset(color_dist_coeff, 0, sizeof(float) * 5);
		depth_extrinsic.setIdentity();
		depth_intrinsic.setIdentity();
		memset(depth_dist_coeff, 0, sizeof(float) * 5);
	}

};

// "Public" interface
class Calibration
{
public:
	Calibration() {
		m_graphics = new D3D11GraphicsDevice();
		m_graphics->initWithoutWindow();
	}
	~Calibration() {
		SAFE_DELETE(m_graphics);
	}


	void Calibration::calibrateScan(const std::string& inSensFilename, const std::string &outSensFilename, const std::string& parametersFilename, const std::string& undistortTableFilename)
	{
		if (!util::fileExists(inSensFilename)) {
			if (util::fileExists(outSensFilename)) {
				std::cout << "output sens file " << outSensFilename << " already exists, skipping" << std::endl;
				return;
			}
			else {
				throw MLIB_EXCEPTION("no sens file: " + inSensFilename);
			}
		}
		if (!util::fileExists(parametersFilename) || !util::fileExists(undistortTableFilename)) throw MLIB_EXCEPTION("no calibration param file(s): " + parametersFilename + " / " + undistortTableFilename);

		// Read in lookup table
		Grid3D undistortTable(undistortTableFilename);


		// Read in parameters
		Calib cd(parametersFilename);

		// Read in sens file
		std::cout << "loading .sens file " << inSensFilename << "... ";
		SensorData sd(inSensFilename);
		std::cout << "done!" << std::endl;

		if (sd.m_calibrationDepth.m_extrinsic == mat4f::identity()) {
			if (inSensFilename != outSensFilename) util::moveFile(inSensFilename, outSensFilename);
			std::cout << "color and depth is already aligned -- cannot further calibrate .sens file -> exiting" << std::endl;
			return;
		}

		calibrateScan(sd, cd, undistortTable);

		// Updating .sens file meta data
		sd.m_sensorName = sd.m_sensorName + " (calibrated)";
		sd.m_calibrationColor.m_extrinsic.setIdentity();
		sd.m_calibrationColor.m_intrinsic = cd.color_intrinsic;
		sd.m_calibrationDepth.m_extrinsic.setIdentity();
		sd.m_calibrationDepth.m_intrinsic = sd.m_calibrationColor.m_intrinsic;

		sd.m_calibrationDepth.m_intrinsic(0, 0) *= (float)sd.m_depthWidth / (float)sd.m_colorWidth;
		sd.m_calibrationDepth.m_intrinsic(1, 1) *= (float)sd.m_depthHeight / (float)sd.m_colorHeight;
		sd.m_calibrationDepth.m_intrinsic(0, 2) *= (float)(sd.m_depthWidth - 1) / (float)(sd.m_colorWidth - 1);
		sd.m_calibrationDepth.m_intrinsic(1, 2) *= (float)(sd.m_depthHeight - 1) / (float)(sd.m_colorHeight - 1);


		// write sens file
		std::cout << "saving .sens file " << outSensFilename << "... ";
		sd.saveToFile(outSensFilename);
		if (inSensFilename != outSensFilename) util::deleteFile(inSensFilename);
		std::cout << "done!" << std::endl;
	}

private:

	DepthImage32 depthToColor(const DepthImage32& input, const Calib& cb) {
		DepthImage32 res;
#pragma omp critical
		{
			Aligner aligner(*m_graphics);
			res = aligner.depthToColor(input, cb.depth_intrinsic, cb.depth_extrinsic, cb.color_intrinsic, cb.color_width, cb.color_height);
		}
		return res;
	}

	//projects a depth image into color space
	DepthImage32 depthToColorDebug(const DepthImage32& input, const Calib& cb) {

		mat4f depthIntrinsicInv = cb.depth_intrinsic.getInverse();
		const mat4f& extrinsic = cb.depth_extrinsic;
		const mat4f& colorIntrinsic = cb.color_intrinsic;

		float scalarWidth = (float)(input.getWidth()-1) / (float)(cb.color_width-1);
		float scalarHeight = (float)(input.getHeight()-1) / (float)(cb.color_height-1);

		DepthImage32 res(input.getWidth(), input.getHeight());
		res.setInvalidValue(input.getInvalidValue());
		res.setPixels(input.getInvalidValue());

		for (auto& o : input) {
			float d = o.value;
			if (d != res.getInvalidValue()) {
				vec3f p = depthIntrinsicInv*vec3f((float)o.x*d, (float)o.y*d, d);	//back-project to camera space
				p = extrinsic * p;													//project to color frame
				vec3f colorCoord = colorIntrinsic * p;								//project to color image space
				colorCoord.x /= colorCoord.z;	colorCoord.y /= colorCoord.z;
				vec2i colorCoordi = math::round(colorCoord);	//use that to get color values
				colorCoordi.x = math::round(colorCoord.x * scalarWidth);
				colorCoordi.y = math::round(colorCoord.y * scalarHeight);

				if (colorCoordi.x >= 0 && colorCoordi.x < (int)res.getWidth() && colorCoordi.y >= 0 && colorCoordi.y < (int)res.getHeight()) {
					res(colorCoordi.x, colorCoordi.y) = colorCoord.z;
				}
			}
		}
		return res;
	}

	template <typename T> BaseImage<T> undistort(const BaseImage<T>& src, const mat4f& intrinsic, const float coeff[5])
	{
		BaseImage<T> res(src.getWidth(), src.getHeight());
		res.setInvalidValue(src.getInvalidValue());
		res.setPixels(res.getInvalidValue());

		for (unsigned int y = 0; y < src.getHeight(); y++)	{
			for (unsigned int x = 0; x < src.getWidth(); x++)	{
				vec2f nic_loc;
				vec2f sample_loc;

				//Normalized image coords
				nic_loc.x = (x - intrinsic(0, 2)) / intrinsic(0, 0);
				nic_loc.y = (y - intrinsic(1, 2)) / intrinsic(1, 1);

				float r2 = nic_loc.x * nic_loc.x + nic_loc.y * nic_loc.y;

				// Radial distortion
				sample_loc.x = nic_loc.x * (1.0f + r2 * coeff[0] + r2*r2 * coeff[1] + r2*r2*r2 * coeff[4]);
				sample_loc.y = nic_loc.y * (1.0f + r2 * coeff[0] + r2*r2 * coeff[1] + r2*r2*r2 * coeff[4]);

				// Tangential distortion
				sample_loc.x += 2.0f * coeff[2] * nic_loc.x * nic_loc.y + coeff[3] * (r2 + 2.0f * nic_loc.x * nic_loc.x);
				sample_loc.y += coeff[2] * (r2 + 2.0f * nic_loc.y * nic_loc.y) + 2.0f * coeff[3] * nic_loc.x * nic_loc.y;

				// Move back to the image space
				sample_loc.x = sample_loc.x * intrinsic(0, 0) + intrinsic(0, 2);
				sample_loc.y = sample_loc.y * intrinsic(1, 1) + intrinsic(1, 2);

				//TODO CONTINUE HERE
				vec2i sample_loc_i = math::round(sample_loc);
				if (src.isValidCoordinate(sample_loc_i)) {
					res(x, y) = src(sample_loc_i);
				}
			}
		}
		return res;
	}


	void undistortDistance(unsigned short * depthData, const SensorData &sd, const Grid3D& undistortTable) {

		// Prepare storage
		// Useful vars
		int w = sd.m_depthWidth;
		int h = sd.m_depthHeight;
		int nSlices = undistortTable.ZRes();
		float xBin = (float)(w / undistortTable.XRes());
		float yBin = (float)(h / undistortTable.YRes());
		float zBin = undistortTable.ZRes() / undistortTable.MaxDist();
		float depthShift = sd.m_depthShift;

		for (int j = 0; j < h; ++j)
		{
			for (int i = 0; i < w; ++i)
			{
				float depth = depthData[j * w + i] / depthShift;
				float zIdx = std::min((float)depth * zBin, (float)nSlices - 1.0f);

				float multiplier = 1.0f / undistortTable.GetValue((float)i / xBin, (float)j / yBin,	zIdx);
				float newDepth = depth * multiplier;
				depthData[j*w + i] = (unsigned short)(newDepth * depthShift);

			}
		}
	}


	void calibrateScan(SensorData& sd, const Calib& cd, const Grid3D& undistortTable) {
		if (cd.depth_width != sd.m_depthWidth || cd.depth_height != sd.m_depthHeight) throw MLIB_EXCEPTION("image dimensions do not match with calibration");

#pragma omp parallel for
		for (int i = 0; i < (int)sd.m_frames.size(); i ++) {
			auto& f = sd.m_frames[i];
			if (omp_get_thread_num() == 0) {
				std::cout << "\rcalibrateScan frame [ " << i*omp_get_num_threads() << " | " << sd.m_frames.size() << " ] ";
			}

			// apply un-distortion to color
			vec3uc* color = sd.decompressColorAlloc(f);
			ColorImageR8G8B8 c(sd.m_colorWidth, sd.m_colorHeight, color);
			c.setInvalidValue(vec3uc(0, 0, 0));
			c = undistort(c, cd.color_intrinsic, cd.color_dist_coeff);
			std::free(color);
			sd.replaceColor(f, c.getData());


			unsigned short* depth = sd.decompressDepthAlloc(f);			
			undistortDistance(depth, sd, undistortTable);	// apply un-distortion based on distance
			DepthImage32 d(sd.m_depthWidth, sd.m_depthHeight);
			d.setInvalidValue(0.0f);
			for (auto& v : d) {
				unsigned int idx = v.y*sd.m_depthWidth + v.x;
				v.value = (float)depth[idx] / sd.m_depthShift;
			}

			// apply barell un-distortion to depth
			d = (DepthImage32)Calibration::undistort(d, cd.depth_intrinsic, cd.depth_dist_coeff);
			// align depth to color			
			d = Calibration::depthToColor(d, cd);	

			// invalidate depth where we have no color
			float scalarWidth = (float)(d.getWidth()-1) / (float)(sd.m_colorWidth-1);
			float scalarHeight = (float)(d.getHeight()-1) / (float)(sd.m_colorHeight-1);

			for (auto& v : d) {
				int x = math::round(v.x / scalarWidth);
				int y = math::round(v.y / scalarHeight);
				if (c(x,y) == vec3uc(0, 0, 0)) {
					v.value = d.getInvalidValue();
				}
			}

			// convert back to u16
			for (auto& v : d) {
				unsigned int idx = v.y*(size_t)sd.m_depthWidth + v.x;
				depth[idx] = math::round(v.value * sd.m_depthShift);
			}
			sd.replaceDepth(f, depth);

			std::free(depth);
		}
		std::cout << std::endl;
	}

	D3D11GraphicsDevice* m_graphics;
};