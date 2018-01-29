# ScanNet iOS Scanner App
The scanner app acquires RGB-D scans using a [structure.io sensor](https://structure.io), stores color, depth and IMU data on local memory and then uploads to the data staging server for processing.

## Prerequisites for compilation
- iPad Air 2 (can work with other iPads and iPhones but untested)
- Xcode 7.2 (get from app store or https://developer.apple.com/download/more/ -- newer Xcode versions work but may require project file updates)
- Included external dependencies: [Structure.io sensor SDK](https://developer.structure.io/sdk) and [GPUImage](https://github.com/BradLarson/GPUImage).

## Build
- Open Scanner.xcodeproj with Xcode
- Set the URL to the data upload server at the top of `Scanner/Config.h`
- Attach your iOS device and authorize the development machine to build to the device
- Build the Scanner target for your device (select "Scanner" and your attached device name at the top left next to the "play" icon, and click the "play" icon)
- Detach the device from the development machine, attach the structure sensor to the device, and run the Scanner app

## Data Formats

**Metadata file (`*.txt`)**:
```
colorWidth = 1296
colorHeight = 968
depthWidth = 640
depthHeight = 480
fx_color = 1170.187988
fy_color = 1170.187988
mx_color = 647.750000
my_color = 483.750000
fx_depth = 571.623718
fy_depth = 571.623718
mx_depth = 319.500000
my_depth = 239.500000
colorToDepthExtrinsics = 0.999977 0.004401 0.005230 -0.037931 -0.004314 0.999852 -0.016630 -0.003321 -0.005303 0.016607 0.999848 -0.021860 -0.000000 0.000000 -0.000000 1.000000
deviceId = AA408AE6-80BB-4E45-B6BA-5ECC8C17FB2F
deviceName = iPad One
sceneLabel = 0001
sceneType = Bedroom / Hotel
numDepthFrames = 1912
numColorFrames = 1912
numIMUmeasurements = 4185
```

**Depth data (`*.depth`)**:
Compressed stream of depth frames from Structure.io sensor.  Please refer to the [depth2pgm](depth2pgm) code for an example of how to parse the data.

**Color data (`*.h264`)**:
An H.264 encoded stream of color frames from the iPad camera.  Can be converted to sequences of images using ffmpeg through commands such as: `ffmpeg -i id.h264 %6d.color.png`.

**Camera data (`*.cam`)**:
Stream of iPad camera settings containing exposure information.  TODO: Format details

**IMU data (`*.imu`)**:
IMU data is a raw binary format consisting of:
```
--- Repeats approx. 53 times per second ---
[8-byte capture device up-time sec. represented in double precision IEEE 754 little-endian]
[8-byte x rotation rate (Radians/s, double, RHR)]
[8-byte y rotation rate (Radians/s, double, RHR)]
[8-byte z rotation rate (Radians/s, double, RHR)]
[8-byte x user-caused acceleration vector (G, double, device's reference frame)]
[8-byte y user-caused acceleration vector (G, double, device's reference frame)]
[8-byte z user-caused acceleration vector (G, double, device's reference frame)]
[8-byte x magnetic field (microtesla, double)]
[8-byte y magnetic field (microtesla, double)]
[8-byte z magnetic field (microtesla, double)]
[8-byte roll (Radians, double)]
[8-byte pitch (Radians, double)]
[8-byte yaw (Radians, double)]
[8-byte x gravity vector (G, double, device's reference frame)]
[8-byte y gravity vector (G, double, device's reference frame)]
[8-byte z gravity vector (G, double, device's reference frame)]
----- REPEAT ------
```
For more information about the IMU data refer to the iOS (CMDeviceMotion API)[https://developer.apple.com/library/prerelease/ios/documentation/CoreMotion/Reference/CMDeviceMotion_Class/index.html].
