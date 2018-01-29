# ScanNet Data Server

The ScanNet server is broken down into the following components:

1. Upload process used by the iPad to upload the scan and trigger scan processing.  To ensure that scans can be automatically processed, the scans should be placed in a directory with lots of space and accessible to the scanning processor.  Tested on Ubuntu 14.04 Linux machine.
2. Scan processing scripts.  Require a Windows machine with high-end GPU (tested on Windows 10 with GTX Titan X with 12GB of VRAM).
3. Indexing scripts.  Go through scan folders and collate information about the scans.

## Upload scripts

The upload script receives scan files (`.h264`, `.depth`, `.imu`, `.txt`, `.camera`) from the iPad and stores them in a staging area for scan processing.  The files are first placed in the `tmp` directory before being moved into the staging directory.  Uses [flask](http://flask.pocoo.org/) with [gunicorn](http://gunicorn.org/) with 10 worker threads on port 8000.

- `install_deps.sh` - Run this to install python dependencies required for the upload server
- `start_upload_server.sh` - Run this to start the upload server.  The main entry point is at `upload.py` (started automatically by this `.sh` script)
- `wsgi.py` - Web service wrapper for `upload.py`

## Scan processing scripts

The scan processing requires a Windows machine with high-end GPU (tested on Titan X 12GB).  It also relies on these third party tools:
- [MeshLab](http://meshlab.sourceforge.net/) for mesh simplification and processing
- [Mitsuba renderer](http://www.mitsuba-renderer.org) for generating preview images for each scan
- [ImageMagick](http://www.imagemagick.org/), [pngquant](https://pngquant.org), and [optipng](http://optipng.sourceforge.net) for thumbnail generation

TODO: Also have a brief list of our binaries that are needed and pointers to their repository locations

- `start_process_server.py` - Starts the scan processing server. The main entry point is at `process.py`.  Running this starts the upload server on port 5000, simple flask server that only handles one request at a time (will block until scan is processed).
- `scan_processor.py` - Main scan processing script.  Edit to see/change path for tools/applications.
- `mts_render.py` - Script to render preview images of scans (ply -> png). Depends on a Mitsuba renderer installation.
- `thumbnail.sh` - Script for generating thumbnails from rendered views
 
## Indexing scripts

Indexing scripts are used to collate information about the scans and index them.
- `monitor.py` - Web service entry point for monitoring and triggering of indexing of scans.  Run `python monitor.py` to start the monitor server on port 5001 (simple flask server).
- `index.py` - Creates index of scans in a directory and outputs a csv file
- `scripts/index_scannet.sh` - Index both staging and checked scans and updates WebUI db

## Statistics computation scripts

- `compute_annotation_stats.py` - Compute aggregated annotation statistics
- `compute_timings.py` - Compute processing times for scans
- `scripts/combine_stats.py` - Combines statistics with index


## Configuration files

- `scannet.json` - Metadata for ScanNet assets for use with the Scene Toolkit viewer
- `scan_stages.json` - The stages of the scan pipeline (so we can track progress)
- `upload.ini` - Configuration file for upload server
