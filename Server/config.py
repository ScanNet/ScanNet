import os
import util

# Server configuration
DATA_SERVER = 'http://localhost:3030'
TEMP_FOLDER = 'tmp'
STAGING_FOLDER = 'staging'
AUTOPROCESS = False

# General paths to binaries
SCRIPT_DIR = util.getScriptPath()
SOURCE_DIR = os.path.join(SCRIPT_DIR, '..')

# System specific paths for processing server binaries
# TODO these need to be abstracted or refactored
GIT_BASH = 'C:\\Program Files\\Git\\bin\\bash.exe'
TOOLS_DIR = 'C:\\tools'
DATA_DIR = 'E:\\share\\data\\scan-net\\'
MESHLAB_BIN = 'C:\\Program Files\\VCG\\MeshLab\\meshlabserver.exe'
FREESPACE_BIN = 'C:\\code\\scanner-ipad\\FreeSpace\\x64\\Release\\FreeSpace.exe'  # exe to call for computing occupancy grids
IMG_MAGIC_DIR = 'C:\\Program Files\\ImageMagick-7.0.2-Q16'

# where scan data is stored under as subdirs with unique ids
STAGING_FOLDER_LOCAL = os.path.join(DATA_DIR, 'scans', 'staging')
