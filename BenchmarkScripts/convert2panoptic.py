#!/usr/bin/python
#
# Convert to COCO-style panoptic segmentation format (http://cocodataset.org/#format-data).
#

# python imports
from __future__ import print_function, absolute_import, division, unicode_literals
import os
import glob
import sys
import argparse
import json
import numpy as np

# Image processing
from PIL import Image

EVAL_LABELS = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 16, 24, 28, 33, 34, 36, 39]

def splitall(path):
    allparts = []
    while 1:
        parts = os.path.split(path)
        if parts[0] == path:  # sentinel for absolute paths
            allparts.insert(0, parts[0])
            break
        elif parts[1] == path: # sentinel for relative paths
            allparts.insert(0, parts[1])
            break
        else:
            path = parts[0]
            allparts.insert(0, parts[1])
    return allparts

# The main method
def convert2panoptic(scannetPath, outputFolder=None):

    if outputFolder is None:
        outputFolder = scannetPath

    # find files
    search = os.path.join(scannetPath, "*", "instance", "*.png")
    files = glob.glob(search)
    files.sort()
    # quit if we did not find anything
    if not files:
        print(
            "Did not find any files for using matching pattern {}. Please consult the README.".format(search)
        )
        sys.exit(-1)
    # a bit verbose
    print("Converting {} annotation files.".format(len(files)))

    outputBaseFile = "scannet_panoptic"
    outFile = os.path.join(outputFolder, "{}.json".format(outputBaseFile))
    print("Json file with the annotations in panoptic format will be saved in {}".format(outFile))
    panopticFolder = os.path.join(outputFolder, outputBaseFile)
    if not os.path.isdir(panopticFolder):
        print("Creating folder {} for panoptic segmentation PNGs".format(panopticFolder))
        os.mkdir(panopticFolder)
    print("Corresponding segmentations in .png format will be saved in {}".format(panopticFolder))

    images = []
    annotations = []
    for progress, f in enumerate(files):

        originalFormat = np.array(Image.open(f))
        
        parts = splitall(f)        
        fileName = parts[-1]
        sceneName = parts[-3]
        outputFileName = "{}__{}".format(sceneName, fileName)
        inputFileName = os.path.join(sceneName, "color", fileName)
        imageId = os.path.splitext(outputFileName)[0]
        # image entry, id for image is its filename without extension
        images.append({"id": imageId,
                       "width": int(originalFormat.shape[1]),
                       "height": int(originalFormat.shape[0]),
                       "file_name": inputFileName})

        pan_format = np.zeros(
            (originalFormat.shape[0], originalFormat.shape[1], 3), dtype=np.uint8
        )
        segmentIds = np.unique(originalFormat)
        segmInfo = []
        for segmentId in segmentIds:
            isCrowd = 0
            if segmentId < 1000:
                semanticId = segmentId
            else:
                semanticId = segmentId // 1000
            if semanticId not in EVAL_LABELS:
                continue

            mask = originalFormat == segmentId
            color = [segmentId % 256, segmentId // 256, segmentId // 256 // 256]
            pan_format[mask] = color

            area = np.sum(mask) # segment area computation

            # bbox computation for a segment
            hor = np.sum(mask, axis=0)
            hor_idx = np.nonzero(hor)[0]
            x = hor_idx[0]
            width = hor_idx[-1] - x + 1
            vert = np.sum(mask, axis=1)
            vert_idx = np.nonzero(vert)[0]
            y = vert_idx[0]
            height = vert_idx[-1] - y + 1
            bbox = [int(x), int(y), int(width), int(height)]

            segmInfo.append({"id": int(segmentId),
                            "category_id": int(semanticId),
                            "area": int(area),
                            "bbox": bbox,
                            "iscrowd": isCrowd})

        annotations.append({'image_id': imageId,
                            'file_name': outputFileName,
                            "segments_info": segmInfo})

        Image.fromarray(pan_format).save(os.path.join(panopticFolder, outputFileName))

        print("\rProgress: {:>3.2f} %".format((progress + 1) * 100 / len(files)), end=' ')
        sys.stdout.flush()

    print("\nSaving the json file {}".format(outFile))
    d = {'images': images,
        'annotations': annotations,
        'categories': categories}
    with open(outFile, 'w') as f:
        json.dump(d, f, sort_keys=True, indent=4)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--dataset-folder",
                        dest="scannetPath",
                        help="path to the ScanNet data 'scenes_all' folder",
                        required=True,
                        type=str)
    parser.add_argument("--output-folder",
                        dest="outputFolder",
                        help="path to the output folder.",
                        default=None,
                        type=str)
    args = parser.parse_args()

    convert2panoptic(args.scannetPath, args.outputFolder)


# call the main
if __name__ == "__main__":
    main()
