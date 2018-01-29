

## ProjectAnnotations

For an RGB-D scan, projects 3d mesh annotations into 2d frames according to the camera trajectory of the sequence.  
See the parameter file `zParametersScan.txt` for input arguments.

### Installation
The code was developed under VS2013.

Requirements:
- DirectX SDK June 2010
- our research library mLib, a git submodule in ../external/mLib
- mLib external libraries can be downloaded [here](https://www.dropbox.com/s/fve3uen5mzonidx/mLibExternal.zip?dl=0)


## Filter2dAnnotations

Perform some basic image filtering on the raw annotation projections from `ProjectAnnotations`.
Fill in the input paths accordingly in the main file under 'Fill in the paths accordingly here'.

### Installation
The code was developed under VS2013.

Requirements:
- NVIDIA CUDA 8.0
- our research library mLib, a git submodule in ../external/mLib
- mLib external libraries can be downloaded [here](https://www.dropbox.com/s/fve3uen5mzonidx/mLibExternal.zip?dl=0)
