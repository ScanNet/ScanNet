# depth2pgm

This example code extracts depth frame data from a `*.depth` file produced by the ScannerApp.

Code compiles by running `make`.

Usage: `depth2pgm input.depth output_basename numDepthFramesToExtract`

Output depth frames are saved in binary PGM format encoding per-pixel depth in mm.
