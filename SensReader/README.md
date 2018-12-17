# Sens File Format (see [sensorData.h](c++/src/sensorData.h)):
```
struct SensorData {
	unsigned int	m_versionNumber;
	std::string	m_sensorName;
	
	CalibrationData m_calibrationColor;		//4x4 intrinsic matrix 
	CalibrationData m_calibrationDepth;		//4x4 intrinsic matrix
	
	COMPRESSION_TYPE_COLOR m_colorCompressionType;	//scannet color frames are typically jpeg encoded
	COMPRESSION_TYPE_DEPTH m_depthCompressionType;	//scannet depth frames are typically zLib encoded
	
	unsigned int m_colorWidth;
	unsigned int m_colorHeight;
	unsigned int m_depthWidth;
	unsigned int m_depthHeight;
	float m_depthShift;				//conversion from float[m] to ushort (typically 1000.0f)
	
	std::vector<RGBDFrame> m_frames;		// <= Main data (contains timestamps and rigid transforms)
	std::vector<IMUFrame> m_IMUFrames;
}

// example get color frame, depth frame, pose (camera to world)
SensorData sd("sensfile.sens");
unsigned int frame = 0;
vec3uc* color = sd.decompressColorAlloc(frame);		// note: must be freed after use
unsigned short* depth = sd.decompressDepthAlloc(frame);	// note: must be freed after use
const mat4f cameraToWorld = sd.m_frames[frame].getCameraToWorld();
std::free(color);
std::free(depth);
// if using mlib, can save a frame to a point cloud with:
sd.saveToPointCloud("frame" + std::to_string(frame) + ".ply", frame);
```

## C++ ToolKit
See [c++/](c++)

## Simple Python Data Exporter
See [python/](python)
