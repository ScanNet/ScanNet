# ScanNet Benchmark Tasks

Train/val/test split for ScanNet is given in [Benchmark](Benchmark).
Label mappings (`scannet-labels.combined.tsv`) can be downloaded with the ScanNet task data release. Download with along with the task data (`--task_data`) or by itself (`--label_map`). ScanNet label ids listed under `id`.

Trained models can be downloaded with the ScanNet data release.

## Installation
Training tasks use [Torch7](http://torch.ch/docs/getting-started.html), with torch packages `cudnn`, `cunn`, `hdf5`, `xlua`.

Train/test data from the ScanNet release should be placed into a `data` folder in the `torch` folder of the respective task, e.g., for 3D Object Classification, unzip the `obj_classification.zip` into `Tasks/ObjectClassification/torch/data`.
(Or edit the paths in the `[phase]_shape_voxel_data_list.txt` and input the train/test paths to `train.lua`.)

Code to generate the training data from ScanNet data to come soon.

## Tasks

### 3D Object Classification

Classification of partially-scanned objects, voxelized to 30x30x30. We use the [3D CNN framework](https://github.com/charlesq34/3dcnn.torch) of [Qi et. al.](https://arxiv.org/abs/1604.03265) with an additional data channel encoding known/unknown voxels according to the camera scanning trajectory. The object classes used are given in [classes_ObjClassification-ShapeNetCore55.txt](Benchmark/classes_ObjClassification-ShapeNetCore55.txt), using ShapeNetCore55 label ids.

* Training:  
 run
 ```
 th train_partial.lua --train_data [path to train h5 file list] --test_data [path to test h5 file list]
 ```
 with the appropriate paths to the train/test data (use '--help' to see more options)
 
* Citation:  
```
@article{qi2016volumetric,
    title={Volumetric and Multi-View CNNs for Object Classification on 3D Data},
    author={Charles Ruizhongtai Qi and Hao Su and Matthias Nie{\ss}ner and Angela Dai and Mengyuan Yan and Leonidas Guibas},
    journal={Proc. Computer Vision and Pattern Recognition (CVPR), IEEE},
    year={2016}
}
```
 
### 3D Object Retrieval

Nearest neighbor 3d model retrieval based on the features of the 3D object classification model. The code in [ObjectRetrieval](ObjectRetrieval) uses the feature extraction and nearest neighbor code from [3dmodel_feature](https://github.com/charlesq34/3dmodel_feature).

To compute nearest neighbors, first extract the features from both the query and the database by: 
 ```
 th generate_feat.lua --model [model] --h5_list_path [path to h5 file list to generate features for] --partial_data [use this flag if partial data] --file_label_file [assocation from h5 data to names and labels] --output_file [output file (features)] --output_name_file [output file (names)] --classes_file [optional, defines set of classes to consider]
 ```
then compute the nearest neighbors with:
 ```
 python nearest_neighbor.py --query_feats [feature file from generate_feat] --database_feats [feature file from generate_feat] --database_models [names corresponding to database feats] --output [output file] 
 ```

### Semantic Voxel Labeling

Prediction of class labels per voxel for voxels on the surface of a scan. Voxels of a scan are labeled column by column using the surrounding neighborhood information (31x31x62 neighborhood for labeling the 1x1x62 center column). The object classes used for ScanNet are given in [classes_SemVoxLabel-nyu40id.txt](Benchmark/classes_SemVoxLabel-nyu40id.txt), using nyu40 label ids.

* Training:  
 ```
 th train.lua --train_data [path to train h5 file list] --test_data [path to test h5 file list]
 ```
 with the appropriate paths to the train/test data (use '--help' to see more options)
 
