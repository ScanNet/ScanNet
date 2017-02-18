# ScanNet

ScanNet is an RGB-D video dataset containing 2.5 million views in more than 1500 scans, annotated with 3D camera poses, surface reconstructions, and instance-level semantic segmentations.

## ScanNet Data

If you would like to download the ScanNet data, please fill out an agreement to the [ScanNet Terms of Use](http://dovahkiin.stanford.edu/scannet-public/ScanNet_TOS.pdf) and send it to us at scannet@googlegroups.com

### Data Organization
The data in ScanNet is organized by RGB-D sequence. Each sequence is stored under a directory with a unique id containing the timestamp and device id.  The raw data captured during scanning, camera poses and surface mesh reconstructions, and annotation metadata are all stored together for the given sequence.  The directory has the following structure:
```shell
<scanId>
|-- <scanId>_vh.sens
    RGB-D sensor stream containing color frames, depth frames, camera poses and other data
|-- <scanId>_vh.ply
    High quality reconstructed mesh
|-- <scanId>_vh_clean_2.ply
    Cleaned and decimated mesh for semantic annotations
|-- <scanId>_vh_clean_2.0.010000.segs.json
    Over-segmentation of annotation mesh
|-- <scanId>.aggregation.json
    Aggregated instance-level semantic annotations
```

### Data Formats
The following are overviews of the data formats used in ScanNet:

**Reconstructed surface mesh file (`*.ply`)**:
Binary PLY format mesh with +Z axis in upright orientation.

**RGB-D sensor stream (`*.sens`)**:
Compressed binary format with per-frame color, depth, camera pose and other data.  See [ScanNet C++ Toolkit](#scannet-c-toolkit) for more information and parsing code.

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

### ScanNet C++ Toolkit
Tools for working with ScanNet data.

* [SensReader](SensReader) loads the ScanNet `.sens` data of compressed RGB-D frames, camera intrinsics and extrinsics, and IMU data.

## Benchmark Tasks
We provide code for several scene understanding benchmarks on ScanNet:
* 3D object classification
* 3D object retrieval
* Dense voxel labeling

See [Tasks](Tasks).


## Citation
If you use the ScanNet data or code please cite:
```
@article{dai2017scannet,
    title={ScanNet: Richly-annotated 3D Reconstructions of Indoor Scenes},
    author={Dai, Angela and Chang, Angel X. and Savva, Manolis and Halber, Maciej and Funkhouser, Thomas and Nie{\ss}ner, Matthias},
    journal={arXiv preprint arXiv:1702.04405},
    year={2017}
}
```

## Help
If you have any questions, please contact us at scannet@googlegroups.com


## Changelog


## License
Copyright (c) 2017
