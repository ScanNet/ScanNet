Camera Calibration Tutorial
============================================

## Step 1 - Data Preparation

For a specific device we are expecting two sets of images:
1. Depth-to-Color: Around 20-30 color and infrared images showing calibration grid 
from different angles and distances. 
2. Depth-Distortion: A video sequence of walking from and to a big flat wall with
calibration grid on it. It is important that for all images in this sequence,
the wall covers __entire__ view frustum. Sequence should be saved as jpgs for color
images and 16-bit pngs for depth.

If you are calibrating your camera, this is where you provide your data. We do 
strongly encourage to go through this tutorial and investigate downloaded data.
When providing your own data, the initial state a directory with a calibration
data for a device should have following structure:
~~~~~~~~
calibration_data
├── depth_distortion
│   ├── color
│   └── depth_raw   
├── depth_to_color
│   ├── color
│   └── infrared
└── validate
    ├── color
    ├── depth_raw
    └── validation.conf
~~~~~~~~

-----------

## Step 2 - Calibration of Color and IR
Run matlab Single Camera Calibration App, by typing 
~~~~~~~~
cameraCalibrator
~~~~~~~~
in Matlab's command line.

After app is loaded and you are presented with user interface, create new 
session, by clicking 'New Session' button. Then click 'Add images' button to 
add images from one of the session, color or infrared. This proces will need to 
be repeated for both. For our example, the grid size is 53 mm, but change it
for your grid size.

After images are loaded, simply press the calibrate button (make sure you're
estimating the camera model with 2 distortion coefficients).  
The resulting reprojection error will depend on resolution of the images, but 
for our experiments for color images with resolution 1296x968, the mean error is
0.23px. For infrared images of with resolution 640x480, it is 0.12 px.

After calibration is done, save the calibration sessions into 
__calibration_data__ folder, as __color_session.mat__ and 
__infrared_session.mat__ for color and infrared images respectively.

Note that if you're excluding some images from __color__ (failed grid detection, 
outliers according to optimization, etc.), relating images must be removed from
__infrared__. Simply put you are excluding pairs, not single images.

-----------

## Step 3 - Parameters Export
From scripts folder run script 
~~~~~~~~
export_calibration_info( <path to calibration_data folder>)
~~~~~~~~
This will create file _parameters.txt_ and other files required for depth 
distortion calibration. Files will be writed into __calibration_data/depth_distortion__ 

-----------

## Step 4 - Depth Distortion Calibration
Assuming you've compiled __calibrate_depth__ app and added it to your PATH, 
you should navigate to __calibration_data/depth_distortion__. Now you should be
able to run: 
~~~~~~~~
calibrate_depth depth_distortion_calibration.conf --estimate_undistortion <name_of_lookup_table.lut>
~~~~~~~~
Note that this is possibly the most timely step. It obviously depends on your machine
and the number of images you are using. On a Early 2013 macbook pro, with XXX images 

Now you have all the files required for applying calibration: __parameters.txt__ and
__<lookup_table.lut>__. __calibrate_depth__ can use these when run in apply mode:
~~~~~~~~
calibrate_depth depth_distortion_calibration.conf --apply_undistortion <name_of_lookup_table.lut>
~~~~~~~~
But this file formats will also work with [ScanNet](http://link) pipeline.

You can also run the __calibrate_depth__ without any argument to see how the
images got undistorted.

-----------

## Step 5 - Validation
Since calibration is a bit of an arcane art, it is often good idea to validate
the results. To do so it is good to prepare some image pairs, that capture
non planar scenes, storing images in a hierarchy as seen in __validate__ folder above.
Then you can run:
~~~~~~~~
calibrate_depth validation.conf --apply_undistortion <path_to_lookup_table.lut>
calibration_explorer ../depth_distortion/parameters.txt color/<color_filename.jpg> depth/<depth_filename.jpg>
~~~~~~~~
You will be able to see color image overlayed with depth_image. It is possible
to manually control alpha, as well as switch to 3D view to investigate how 
well images are aligned.

All calibration parameters are also exposed if some manual intervention is needed/wanted.
You can pass __--output_parameters <parameters_filename.txt>__ to the 
__calibration_explorer__ to be able to save the modified parameters.
