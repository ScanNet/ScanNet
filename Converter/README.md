## Converter from ScannerApp capture to .sens data
===============================================

Converts raw capture data (`.h264`, `.depth`, `.txt`, `.imu`) output from the ScannerApp to `.sens` format.

### Installation.
This code was developed under VS2013.

Requirements:
- ffmpeg (assumes that `ffmpeg.exe` is at the relativel path `./ffmpeg/ffmpeg.exe`)
- our research library mLib, a git submodule in ../external/mLib
- mLib external libraries can be downloaded [here](https://www.dropbox.com/s/fve3uen5mzonidx/mLibExternal.zip?dl=0)


To run:  
`converter.exe [path to directory of ScannerApp outputs] [name of ScannerApp output to convert]`
