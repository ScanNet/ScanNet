# Example script to convert instance images from the *_2d-instance.zip or *_2d-instance-filt.zip data for each scan.
# Note: already preprocessed data for a set of frames subsampled from the full datasets is available to download through the ScanNet download.
# Input:
#   - path to instance image to convert
#   - path to the corresponding label image
#   - label mapping file (scannetv2-labels.combined.tsv)
#   - output image file
# Outputs the instance image with each pixel represented by label * 1000 + instance_id as an 16-bit image
# Labels are mapped to the nyu40 set, and walls/floors/ceilings are not assigned instances (instance_id = 0) 
#
# example usage: convert_scannet_instance_image.py --input_instance_file [path to input instance image] --input_label_file [path to corresponding label image] --label_map_file [path to scannetv2-labels.combined.tsv] --output_file [output image file]
# (see util.visualize_instance_image() for producing a colored visualization)


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
parser.add_argument('--input_instance_file', required=True, help='path to input instance image')
parser.add_argument('--input_label_file', required=True, help='path to corresponding label image')
parser.add_argument('--label_map_file', required=True, help='path to scannet-labels.combined.tsv')
parser.add_argument('--output_file', required=True, help='output image file')
opt = parser.parse_args()


def map_label_image(image, label_mapping):
    mapped = np.copy(image)
    for k,v in label_mapping.iteritems():
        mapped[image==k] = v
    return mapped.astype(np.uint8)


def make_instance_image(label_image, instance_image):
    output = np.zeros_like(instance_image, dtype=np.uint16)
    # oldinst2labelinst = {}
    label_instance_counts = {}
    old_insts = np.unique(instance_image)
    for inst in old_insts:
        label = label_image[instance_image==inst][0]
        if label in label_instance_counts:
            inst_count = label_instance_counts[label] + 1
            label_instance_counts[label] = inst_count
        else:
            inst_count = 1
            label_instance_counts[label] = inst_count
        # oldinst2labelinst[inst] = (label, inst_count)
        output[instance_image==inst] = label * 1000 + inst_count
    return output


def get_labels_from_instance(instance_image):
    labels = instance_image // 1000
    return labels.astype(np.uint8)


def main():
    instance_image = np.array(imageio.imread(opt.input_instance_file))
    label_image = np.array(imageio.imread(opt.input_label_file))
    label_map = util.read_label_mapping(opt.label_map_file, label_from='id', label_to='nyu40id')
    mapped_label = map_label_image(label_image, label_map)
    output_instance_image = make_instance_image(mapped_label, instance_image)
    imageio.imwrite(opt.output_file, output_instance_image)
    # uncomment to save out visualization
    #util.visualize_instance_image(os.path.splitext(opt.output_file)[0] + '_vis.jpg', output_instance_image)
    #util.visualize_instance_image(os.path.splitext(opt.output_file)[0] + '_vis-labels.jpg', get_labels_from_instance(output_instance_image))


if __name__ == '__main__':
    main()


