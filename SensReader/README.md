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

class RGBDFrame {
	UINT64 m_colorSizeBytes;			//compressed byte size
	UINT64 m_depthSizeBytes;			//compressed byte size
	unsigned char* m_colorCompressed;		//compressed color data
	unsigned char* m_depthCompressed;		//compressed depth data
	UINT64 m_timeStampColor;			//time stamp color (convection: in microseconds)
	UINT64 m_timeStampDepth;			//time stamp depth (convention: in microseconds)
	mat4f m_cameraToWorld;				//camera trajectory: from current frame to base(world) frame
}

struct IMUFrame {
	vec3d rotationRate;				//angular velocity (raw data)
	vec3d acceleration;				//acceleration in x,y,z direction (raw data)
	vec3d magneticField;				//magnetometer data (raw data)
	vec3d attitude;					//roll, pitch, yaw estimate (inferred)			
	vec3d gravity;					//gravitation dir estimate (inferred)	
	UINT64 timeStamp;				//timestamp (typically in microseconds)
}
```

## C++ ToolKit
See [c++/](c++)

## Simple Python Data Exporter
See [python/](python)
