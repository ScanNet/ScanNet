# Example script to convert label images from the *_2d-label.zip or *_2d-label-filt.zip data for each scan.
# Note: already preprocessed data for a set of frames subsampled from the full datasets is available to download through the ScanNet download.
# Input:
#   - path to label image to convert
#   - label mapping file (scannetv2-labels.combined.tsv)
#   - output image file
# Outputs the label image with nyu40 labels as an 8-bit image 
#
# example usage: convert_scannet_label_image.py --input_file [path to input label image] --label_map_file [path to scannet-labels.combined.tsv] --output_file [output image file]
# (see util.visualize_label_image() for producing a colored visualization)


# python imports
import math
import os, sys, argparse
import inspect

try:
    import numpy as np
except:
    print "Failed to import numpy package."
    sys.exit(-1)
try:
    import imageio
except:
    print("Please install the module 'imageio' for image processing, e.g.")
    print("pip install imageio")
    sys.exit(-1)

currentdir = os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))
parentdir = os.path.dirname(currentdir)
sys.path.insert(0,parentdir)
import util

parser = argparse.ArgumentParser()
parser.add_argument('--input_file', required=True, help='path to input label image')
parser.add_argument('--label_map_file', required=True, help='path to scannetv2-labels.combined.tsv')
parser.add_argument('--output_file', required=True, help='output image file')
opt = parser.parse_args()


def map_label_image(image, label_mapping):
    mapped = np.copy(image)
    for k,v in label_mapping.iteritems():
        mapped[image==k] = v
    return mapped.astype(np.uint8)


def main():
    image = np.array(imageio.imread(opt.input_file))
    label_map = util.read_label_mapping(opt.label_map_file, label_from='id', label_to='nyu40id')
    mapped_image = map_label_image(image, label_map)
    imageio.imwrite(opt.output_file, mapped_image)
    # uncomment to save out visualization
    # util.visualize_label_image(os.path.splitext(opt.output_file)[0] + '_vis.jpg', mapped_image)


if __name__ == '__main__':
    main()


