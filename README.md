# ScanNet

ScanNet is an RGB-D video dataset containing 2.5 million views in more than 1500 scans, annotated with 3D camera poses, surface reconstructions, and instance-level semantic segmentations.

## ScanNet Data

If you would like to download the ScanNet data, please fill out an agreement to the [ScanNet Terms of Use](http://kaldir.vc.in.tum.de/scannet/ScanNet_TOS.pdf), using your institutional email addresses, and send it to us at scannet@googlegroups.com. 

If you have not received a response within a week, it is likely that your email is bouncing - please check this before sending repeat requests. Please do not reply to the noreply email - your email won't be seen.

Please check the [changelog](http://www.scan-net.org/changelog) for updates to the data release.


### Data Organization
The data in ScanNet is organized by RGB-D sequence. Each sequence is stored under a directory with named `scene<spaceId>_<scanId>`, or `scene%04d_%02d`, where each space corresponds to a unique location (0-indexed).  The raw data captured during scanning, camera poses and surface mesh reconstructions, and annotation metadata are all stored together for the given sequence.  The directory has the following structure:
```shell
<scanId>
|-- <scanId>.sens
    RGB-D sensor stream containing color frames, depth frames, camera poses and other data
|-- <scanId>_vh_clean.ply
    High quality reconstructed mesh
|-- <scanId>_vh_clean_2.ply
    Cleaned and decimated mesh for semantic annotations
|-- <scanId>_vh_clean_2.0.010000.segs.json
    Over-segmentation of annotation mesh
|-- <scanId>.aggregation.json, <scanId>_vh_clean.aggregation.json
    Aggregated instance-level semantic annotations on lo-res, hi-res meshes, respectively
|-- <scanId>_vh_clean_2.0.010000.segs.json, <scanId>_vh_clean.segs.json
    Over-segmentation of lo-res, hi-res meshes, respectively (referenced by aggregated semantic annotations)
|-- <scanId>_vh_clean_2.labels.ply
    Visualization of aggregated semantic segmentation; colored by nyu40 labels (see img/legend; ply property 'label' denotes the nyu40 label id)
|-- <scanId>_2d-label.zip
    Raw 2d projections of aggregated annotation labels as 16-bit pngs with ScanNet label ids
|-- <scanId>_2d-instance.zip
    Raw 2d projections of aggregated annotation instances as 8-bit pngs
|-- <scanId>_2d-label-filt.zip
    Filtered 2d projections of aggregated annotation labels as 16-bit pngs with ScanNet label ids
|-- <scanId>_2d-instance-filt.zip
    Filtered 2d projections of aggregated annotation instances as 8-bit pngs
```

### Data Formats
The following are overviews of the data formats used in ScanNet:

**Reconstructed surface mesh file (`*.ply`)**:
Binary PLY format mesh with +Z axis in upright orientation.

**RGB-D sensor stream (`*.sens`)**:
Compressed binary format with per-frame color, depth, camera pose and other data.  See [ScanNet C++ Toolkit](#scannet-c-toolkit) for more information and parsing code. See [SensReader/python](SensReader/python) for a very basic python data exporter.

**Surface mesh segmentation file (`*.segs.json`)**:
```javascript
{
  "params": {  // segmentation parameters
   "kThresh": "0.0001",
   "segMinVerts": "20",
   "minPoints": "750",
   "maxPoints": "30000",
   "thinThresh": "0.05",
   "flatThresh": "0.001",
   "minLength": "0.02",
   "maxLength": "1"
  },
  "sceneId": "...",  // id of segmented scene
  "segIndices": [1,1,1,1,3,3,15,15,15,15],  // per-vertex index of mesh segment
}
```

**Aggregated semantic annotation file (`*.aggregation.json`)**:
```javascript
{
  "sceneId": "...",  // id of annotated scene
  "appId": "...", // id + version of the tool used to create the annotation
  "segGroups": [
    {
      "id": 0,
      "objectId": 0,
      "segments": [1,4,3],
      "label": "couch"
    },
  ],
  "segmentsFile": "..." // id of the *.segs.json segmentation file referenced
}
```
[BenchmarkScripts/util_3d.py](BenchmarkScripts/util_3d.py) gives examples to parsing the semantic instance information from the `*.segs.json`, `*.aggregation.json`, and `*_vh_clean_2.ply` mesh file, with example semantic segmentation visualization in [BenchmarkScripts/3d_helpers/visualize_labels_on_mesh.py](BenchmarkScripts/3d_helpers/visualize_labels_on_mesh.py).

**2d annotation projections (`*_2d-label.zip`, `*_2d-instance.zip`, `*_2d-label-filt.zip`, `*_2d-instance-filt.zip`)**:
Projection of 3d aggregated annotation of a scan into its RGB-D frames, according to the computed camera trajectory. 

### ScanNet C++ Toolkit
Tools for working with ScanNet data. [SensReader](SensReader) loads the ScanNet `.sens` data of compressed RGB-D frames, camera intrinsics and extrinsics, and IMU data.

### Camera Parameter Estimation Code
Code for estimating camera parameters and depth undistortion. Required to compute sensor calibration files which are used by the pipeline server to undistort depth. See [CameraParameterEstimation](CameraParameterEstimation) for details.

### Mesh Segmentation Code
Mesh supersegment computation code which we use to preprocess meshes and prepare for semantic annotation. Refer to [Segmentator](Segmentator) directory for building and using code.

## BundleFusion Reconstruction Code

ScanNet uses the [BundleFusion](https://github.com/niessner/BundleFusion) code for reconstruction. Please refer to the BundleFusion repository at https://github.com/niessner/BundleFusion . If you use BundleFusion, please cite the original paper:
```
@article{dai2017bundlefusion,
  title={BundleFusion: Real-time Globally Consistent 3D Reconstruction using On-the-fly Surface Re-integration},
  author={Dai, Angela and Nie{\ss}ner, Matthias and Zoll{\"o}fer, Michael and Izadi, Shahram and Theobalt, Christian},
  journal={ACM Transactions on Graphics 2017 (TOG)},
  year={2017}
}
```

## ScanNet Scanner iPad App
[ScannerApp](ScannerApp) is designed for easy capture of RGB-D sequences using an iPad with attached Structure.io sensor.

## ScanNet Scanner Data Server
[Server](Server) contains the server code that receives RGB-D sequences from iPads running the Scanner app.

## ScanNet Data Management UI
[WebUI](WebUI) contains the web-based data management UI used for providing an overview of available scan data and controlling the processing and annotation pipeline.

## ScanNet Semantic Annotation Tools
Code and documentation for the ScanNet semantic annotation web-based interfaces is provided as part of the [SSTK](https://github.com/smartscenes/sstk) library. Please refer to https://github.com/smartscenes/sstk/wiki/Scan-Annotation-Pipeline for an overview.

## Benchmark Tasks
We provide code for several scene understanding benchmarks on ScanNet:
* 3D object classification
* 3D object retrieval
* Semantic voxel labeling

Train/test splits are given at [Tasks/Benchmark](Tasks/Benchmark).   
Label mappings and trained models can be downloaded with the ScanNet data release.

See [Tasks](Tasks).

### Labels
The label mapping file (`scannet-labels.combined.tsv`) in the ScanNet task data release contains mappings from the labels provided in the ScanNet annotations (`id`) to the object category sets of [NYUv2](http://cs.nyu.edu/~silberman/datasets/nyu_depth_v2.html), [ModelNet](http://modelnet.cs.princeton.edu/), [ShapeNet](https://www.shapenet.org/), and [WordNet](https://wordnet.princeton.edu/) synsets. Download with along with the task data (`--task_data`) or by itself (`--label_map`).

## Citation
If you use the ScanNet data or code please cite:
```
@inproceedings{dai2017scannet,
    title={ScanNet: Richly-annotated 3D Reconstructions of Indoor Scenes},
    author={Dai, Angela and Chang, Angel X. and Savva, Manolis and Halber, Maciej and Funkhouser, Thomas and Nie{\ss}ner, Matthias},
    booktitle = {Proc. Computer Vision and Pattern Recognition (CVPR), IEEE},
    year = {2017}
}
```

## Help
If you have any questions, please contact us at scannet@googlegroups.com


## Changelog

## License
The data is released under the [ScanNet Terms of Use](http://kaldir.vc.in.tum.de/scannet/ScanNet_TOS.pdf), and the code is released under the MIT license.

Copyright (c) 2017
