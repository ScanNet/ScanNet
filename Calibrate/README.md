## Converts uncalibrated .sens data to color/depth calibrated .sens data
===========================================================================

Converts uncalibrated `.sens` file to calibrated `.sens` file; calibrates color and depth distortion and aligns depth with color space.
Uses the calibration output of [CameraParameterEstimation](../CameraParameterEstimation) for a calibrated device.

### Installation.
This code was developed under VS2013.

Requirements:
- DirectX SDK June 2010
- our research library mLib, a git submodule in ../external/mLib
- mLib external libraries can be downloaded [here](https://www.dropbox.com/s/fve3uen5mzonidx/mLibExternal.zip?dl=0)


To run:  
`calibrate.exe [input sens file] [output sens file] [device calibration map file (from CameraParameterEstimation)] [directory of device calibration map files]`