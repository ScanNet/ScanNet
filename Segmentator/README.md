Mesh Segmentation
=================

Mesh segmentation code using Felzenswalb and Huttenlocher's [*Graph Based Image Segmentation*](https://cs.brown.edu/~pff/segment/index.html) algorithm on computed mesh normals.

Build by running `make` (or create makefiles for your system using `cmake`). This will create a `segmentator` binary that can be run by:

`./segmentator input.ply [kThresh=0.01] [segMinVerts=20]`

The first argument is a path to an input mesh in PLY format.
The second (optional) argument is the segmentation cluster threshold parameter (larger values lead to larger segments).
The third (optional) argument is the minimum number of vertices per-segment, enforced by merging small clusters into larger segments.