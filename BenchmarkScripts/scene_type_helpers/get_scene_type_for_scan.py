# Example script to get the scene type from a scan in the ScanNet release
# Input:
#   - path to the *.txt info file for the scan
#   - path to scene_types.txt or scene_types_all.txt
#
# example usage: get_scene_type_for_scan.py --info_file ./data/ScanNet/v2/scans/scene0000_00/scene0000_00.txt --scene_type_labels_file ./data/ScanNet/v2/tasks/scene_types_all.txt

# python imports
import math
import os, sys, argparse
import inspect

currentdir = os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))
parentdir = os.path.dirname(currentdir)
sys.path.insert(0,parentdir)
import util

parser = argparse.ArgumentParser()
parser.add_argument('--info_file', required=True, help='path to the *.txt info file for the scan')
parser.add_argument('--scene_type_labels_file', required=True, help='path to scene_types.txt or scene_types_all.txt')
opt = parser.parse_args()


def get_scene_type_id(type_name, type_mapping):
    name = type_name.strip().lower()
    if name in type_mapping:
        return type_mapping[name]
    return -1


def get_field_from_info_file(filename, field_name):
    lines = open(filename).read().splitlines()
    lines = [line.split(' = ') for line in lines]
    mapping = { x[0]:x[1] for x in lines }
    if field_name in mapping:
        return mapping[field_name]
    else:
        util.print_error('Failed to find %s in info file %s' % (field_name, filename))


def main():
    scene_name = os.path.splitext(os.path.basename(opt.info_file))[0]
    scene_type_mapping = util.read_scene_types_mapping(opt.scene_type_labels_file, remove_spaces=True)
    type_name = get_field_from_info_file(opt.info_file, 'sceneType')
    id = get_scene_type_id(type_name, scene_type_mapping)
    if id == -1:
        print('%s ==> %s ==> (not in scene types list)' % (scene_name, type_name))
    else:
        print('%s ==> %s ==> %d' % (scene_name, type_name, id))


if __name__ == '__main__':
    main()
