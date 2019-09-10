# Data Exporter

Developed and tested with python 2.7.

Usage:
```
python reader.py --filename [.sens file to export data from] --output_path [output directory to export data to]
Options:
--export_depth_images: export all depth frames as 16-bit pngs (depth shift 1000)
--export_color_images: export all color frames as 8-bit rgb jpgs
--export_poses: export all camera poses (4x4 matrix, camera to world)
--export_intrinsics: export camera intrinsics (4x4 matrix)
```
