#!/usr/bin/python
#
# Convert instances from png files to a dictionary
#

from __future__ import print_function
import os, sys
from instance import Instance
from PIL import Image
import numpy as np

def instances2dict(imageFileList, class_labels, class_ids, verbose=False):
    imgCount     = 0
    instanceDict = {}
    label2id = {}
    id2label = {}
    for i in range(len(class_labels)):
         label2id[class_labels[i]] = class_ids[i]
         id2label[class_ids[i]] = class_labels[i]

    if not isinstance(imageFileList, list):
        imageFileList = [imageFileList]

    if verbose:
        print("Processing {} images...".format(len(imageFileList)))

    for imageFileName in imageFileList:
        # Load image
        img = Image.open(imageFileName)

        # Image as numpy array
        imgNp = np.array(img)

        # Initialize label categories
        instances = {}
        for label in class_labels:
            instances[label] = []

        # Loop through all instance ids in instance image
        for instanceId in np.unique(imgNp):
            instanceObj = Instance(imgNp, instanceId)
            if instanceObj.labelID in class_ids:
                instances[id2label[instanceObj.labelID]].append(instanceObj.toDict())

        imgKey = os.path.abspath(imageFileName)
        instanceDict[imgKey] = instances
        imgCount += 1

        if verbose:
            print("\rImages Processed: {}".format(imgCount), end=' ')
            sys.stdout.flush()

    if verbose:
        print("")

    return instanceDict

def main(argv):
    fileList = []
    if (len(argv) > 2):
        for arg in argv:
            if ("png" in arg):
                fileList.append(arg)
    instances2dict(fileList, True)

if __name__ == "__main__":
    main(sys.argv[1:])
