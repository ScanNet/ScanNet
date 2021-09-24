


Example to decode .sens files: 
- spits out each frame as .jpg, .pgm, and .pose.txt
- metadata output in _info.txt (includes intrinsics, etc.)
- IMU data is not saved out (it's stored in m_IMUFrames)

Run:
./sens <sensFile> <outputDir>

Hint: 	keep the sens files as they are a nice represention
		see processFrame(..) to decode independent frames
		
For additional functionality (vector/matrix/point cloud classes, etc), include the mLib library: https://github.com/niessner/mLib.

- tested under Windows10 VS2013
- tested 14.04.1-Ubuntu: g++ and clang


==============================================================

Sens File Format (see sensorData.h):
struct SensorData {
	unsigned int	m_versionNumber;
	std::string		m_sensorName;
	
	CalibrationData m_calibrationColor;			//4x4 intrinsic matrix 
	CalibrationData m_calibrationDepth;			//4x4 intrinsic matrix
	
	COMPRESSION_TYPE_COLOR m_colorCompressionType;	//scannet color frames are typically jpeg encoded
	COMPRESSION_TYPE_DEPTH m_depthCompressionType;	//scannet depth frames are typically zLib encoded
	
	unsigned int m_colorWidth;
	unsigned int m_colorHeight;
	unsigned int m_depthWidth;
	unsigned int m_depthHeight;
	float m_depthShift;	//conversion from float[m] to ushort (typically 1000.0f)
	
	std::vector<RGBDFrame> m_frames;	// <= Main data (contains timestamps and rigid transforms)
	std::vector<IMUFrame> m_IMUFrames;
}


================================================================

Useful functions in sensorData.h:
	vec3uc* = sd.decompressColorAlloc(frameIdx);
	unsigned short* d = sd.decompressDepthAlloc(frameIdx);
	IMUFrame f = sd.findClosestIMUFrame(frameIdx);
	mat4f pose = sd.m_frames[frameIdx].getCameraToWorld();
	
================================================================
Notes:
	The invalid poses are marked with -inf values. They are result of lost tracking.
	Subsequen poses can be trusted, as they are result of global alignment in 
	BundleFusion[Dai et al.] algorithm.
