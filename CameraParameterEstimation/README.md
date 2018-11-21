Structured Light Camera Calibration
=====================================

This repository includes set of Matlab scripts and C++ programs used for
calibration of structured light depth camera. Specifically, we have calibrated 
a set of Structure Sensor Cameras from [Occipital](http://http://occipital.com).

We perform two types of calibration:
  1. Depth-To-Color Calibration
  2. Depth Distortion Calibration

Code for performing the Depth-To-Color calibration depends on Matlab's 
[Single Camera Calibration App](https://www.mathworks.com/help/vision/ug/single-camera-calibrator-app.html)

Below we describe the both calibration types. For detailed step-by-step guide
for calibrating your camera, please see example folder.

-------------------------

## Depth-To-Color
This step's input is a set of 20-30 image pairs, observing a calibration grid
from different angles and distances. For this step we record rgb and infrared
images.

Output is set of intrinsic properties for both color and infrared(depth) cameras,
as well as rigid transformation that aligns depth to color. 

-------------------------

## Depth Distortion Calibration
The depth images returned by Structure Sensor suffer from severe distortion, that
get worse with the distance to the observed object. To undistort such images use of 
a lookup table has been proposed ([Teichman et al. 13](https://pdfs.semanticscholar.org/193c/9974f85ab12636cb9bfdcabd345393c357d4.pdf))

Input to this step is a rgbd video sequence (~400 frames in our experiments) observing a large flat
wall with a calibration grid on it. ( it is important that flat wall covers entire
view frustrum of a camera )

The output is a lookup table. We provide a C++ program that applies estimated
calibration to 16-bit depth images stored as png.

-------------------------

## Validation
We also provide calibration_explorer application that can be used to verify the
calibration results.


