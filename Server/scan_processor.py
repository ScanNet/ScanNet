#!/usr/bin/env python
#
# Process scans from ipad and runs recons pipeline
# Run with ./scan_processor.py (or python scan_processor.py on Windows)

import argparse
import os
import logging

import util

from glob import glob
import config as cfg

PROCESSES = ['convert', 'calibrate', 'recons', 'clean', 'improve', 'decimate', 'freespace', 'segment', 'render', 'thumbnail']


# where devices information is stored
DEVICES_DIR = os.path.join(cfg.DATA_DIR, 'devices')
DEVICES_CSV = os.path.join(DEVICES_DIR, 'devices.csv')
# exe to call for converting iPad raw files to .sens files
CONVERTER_DIR = os.path.join(cfg.TOOLS_DIR, 'converter')
CONVERTER_BIN = os.path.join(CONVERTER_DIR, 'converter.exe')
# exe to call for calibration
CALIBRATE_DIR = os.path.join(cfg.TOOLS_DIR, 'calibrate')
CALIBRATE_BIN = os.path.join(CALIBRATE_DIR, 'calibrate.exe')
# exe to call for reconstructing .sens to .ply etc
RECONS_DIR = os.path.join(cfg.TOOLS_DIR, 'FriedLiver')
RECONS_BIN = os.path.join(RECONS_DIR, 'FriedLiver.exe zParametersScanNet.txt zParametersBundlingScanNet.txt')
# exe to call for aligning .sens to up,x,y directions
ALIGN_DIR = os.path.join(cfg.TOOLS_DIR, 'alignment')
ALIGN_BIN = os.path.join(ALIGN_DIR, 'alignment.exe')
# exe to call for hi-res mesh gen
VOXELHASHING_DIR = os.path.join(cfg.TOOLS_DIR, 'VoxelHashing')
VOXELHASHING_BIN = os.path.join(VOXELHASHING_DIR, 'DepthSensing.exe zParametersScanNet.txt zParametersTrackingDefault.txt')
# for mesh cleaning/simplification
MESHPROC_DIR = os.path.join(cfg.TOOLS_DIR, 'meshclean')
# exe for segmentation
SEGMENT_DIR = os.path.join(cfg.TOOLS_DIR, 'segmentor')
SEGMENT_BIN = os.path.join(SEGMENT_DIR, 'Segmentator.exe')
# exe for rendering
RENDER_DIR = cfg.SCRIPT_DIR
RENDER_BIN = 'python ' + os.path.join(RENDER_DIR, 'mts_render.py')
# exe for thumbnailing
THUMBNAIL_BIN = cfg.GIT_BASH + ' ' + os.path.join(cfg.SCRIPT_DIR, 'scripts', 'thumbnail.sh -v')
IMG_CONVERT_BIN = os.path.join(cfg.IMG_MAGIC_DIR, 'convert')
OPTIPNG_BIN = os.path.join(cfg.TOOLS_DIR, 'optipng', 'optipng')
PNGQUANT_BIN = os.path.join(cfg.TOOLS_DIR, 'pngquant', 'pngquant')


FORMAT = '%(asctime)-15s [%(levelname)s] %(message)s'
formatter = logging.Formatter(FORMAT)
logging.basicConfig(format=FORMAT)
log = logging.getLogger('scan_processor')
log.setLevel(logging.INFO)

TEST_MODE = False


def update_config(config):
    if config.get('all'):
        for name in PROCESSES:
            config[name] = True
    elif config.get('from'):
        fromSeen = False
        for name in PROCESSES:
            fromSeen = fromSeen or name == config['from']
            config[name] = fromSeen
    return config

def process_scan_dir(path, name, config):
    # Check if valid scan dir
    namebase = os.path.join(path, name)
    validNovh = os.path.isfile(namebase + ".ply") and config.get('novh')
    if not (validNovh or os.path.isfile(namebase + ".depth") or os.path.isfile(name + ".sens")):
        msg = 'path %s not a valid scan dir' % path
        log.info(msg)
        return msg

    # Wrapper around process_scan_dir_basic but with logging to file
    fh = logging.FileHandler(os.path.join(path, 'process.log'))
    fh.setLevel(logging.INFO)
    fh.setFormatter(formatter)
    log.addHandler(fh)
    msg = ''
    try:
        msg = process_scan_dir_basic(path, name, config)
        log.info(msg)
    finally:
        log.removeHandler(fh)
        fh.close()
    return msg


def process_scan_dir_basic(path, name, config):
    # Check if already processed
    if not config.get('overwrite'):
        processedFile = os.path.join(path, 'processed.txt')
        if os.path.isfile(processedFile):
            return 'Scan at %s already processed' % path

    # Convert to sens file
    path = os.path.abspath(path)
    outbase = os.path.join(path, name)
    uncalibrated_sensfile = outbase + '.uncalibrated.sens'
    sensfile = outbase + '.sens'

    if config.get('overwrite') or not os.path.isfile(sensfile):
        if config.get('convert'):
            ret = util.call([CONVERTER_BIN, path, uncalibrated_sensfile], log, CONVERTER_DIR, desc='convert')
            if not os.path.isfile(uncalibrated_sensfile) and not TEST_MODE:
                return 'Scan at %s aborted: no uncalibrated sens file (convert failed)' % path

        # Calibrate
        if config.get('calibrate'):
            if not os.path.isfile(uncalibrated_sensfile) and not TEST_MODE:
                return 'Scan at %s aborted: no uncalibrated sens file for calibrate' % path
            ret = util.call([CALIBRATE_BIN, uncalibrated_sensfile, sensfile, DEVICES_CSV, DEVICES_DIR], log, CALIBRATE_DIR, desc='calibrate')
    else:
        log.info(sensfile + ' already exists, skipping convert/calibrate')

    # Reconstruction to produce ply
    if config.get('recons'):
        if not os.path.isfile(sensfile) and not TEST_MODE:
            return 'Scan at %s aborted: no calibrated sens file (calibrate failed)' % path
        ret = util.call(RECONS_BIN + ' ' + sensfile, log, RECONS_DIR, desc='recons')

    # Clean and align ply
    plyfile = outbase + '.ply'
    if not os.path.isfile(plyfile) and not TEST_MODE:
        return 'Scan at %s aborted: no ply (recons failed)' % path
    if config.get('clean'):
        # TODO check: overwrite orig ply file?
        ret = util.call(cfg.MESHLAB_BIN + ' -i ' + plyfile + ' -o ' + plyfile + ' -m vc -s ' + MESHPROC_DIR + 'cleanLoRes.mlx', log, desc='clean1')
        ret = util.call(ALIGN_BIN + ' ' + path, log, ALIGN_DIR, desc='clean2')

    if config.get('improve'):
        ret = util.call(VOXELHASHING_BIN + ' ' + sensfile, log, VOXELHASHING_DIR, desc='improve')

    # Mesh processing
    plybase = outbase if config.get('novh') else outbase + '_vh'
    if config.get('decimate'):
        ret = util.call(cfg.MESHLAB_BIN + ' -i ' + plybase + '.ply -o ' + plybase + '_clean.ply -m vc -s ' + MESHPROC_DIR + 'clean.mlx', log, desc='decimate1')
        ret = util.call(cfg.MESHLAB_BIN + ' -i ' + plybase + '_clean.ply -o ' + plybase + '_clean_1.ply -m vc -s ' + MESHPROC_DIR + 'simplify.mlx', log, desc='decimate2')
        ret = util.call(cfg.MESHLAB_BIN + ' -i ' + plybase + '_clean_1.ply -o ' + plybase + '_clean_2.ply -m vc -s ' + MESHPROC_DIR + 'simplify.mlx', log, desc='decimate3')
    decimated_mesh = plybase + '_clean_2.ply'
    if not os.path.isfile(decimated_mesh) and not TEST_MODE:
        return 'Scan at %s has no downsampled mesh' % path

    # Freespace
    if config.get('freespace'):
        ret = util.call(cfg.FREESPACE_BIN + ' ' + sensfile, log, desc='freespace')

    # Segment
    if config.get('segment'):
        ret = util.call(SEGMENT_BIN + ' ' + decimated_mesh, log, SEGMENT_DIR, desc='segment')

    # Generate images
    if config.get('render'):
        ret = util.call(RENDER_BIN + ' ' + decimated_mesh, log, RENDER_DIR, desc='render')

    # Generate thumbnails
    if config.get('thumbnail'):
        ret = util.call(THUMBNAIL_BIN + ' ' + path.replace('\\', '/'), log, 
            env=dict(os.environ, CONVERT=IMG_CONVERT_BIN, PNGQUANT=PNGQUANT_BIN, OPTIPNG=OPTIPNG_BIN), desc='thumbnail')

    return 'Scan at %s processed' % path


def process_scan_dir_batch(dirname, config):
    # For now, assume one directory deep (can use os.walk() to recursively descend
    entries = glob(dirname + '/*/')
    for dir in entries:
        name = os.path.relpath(dir, dirname)
        log.info('Processing ' + dir + ' ' + name)
        process_scan_dir(dir, name, config)


def process_scan_dirs(dirs, config):
    # For now, assume one directory deep (can use os.walk() to recursively descend
    for dir in dirs:
        name = os.path.relpath(dir, dir + '/..')
        log.info('Processing ' + dir + ' ' + name)
        process_scan_dir(dir, name, config)


def main():
    # Argument processing
    parser = argparse.ArgumentParser(description='Process scans!!!')
    parser.add_argument('-i', '--input', dest='input', action='store',
                        required=True,
                        help='Input directory or list of directories (if file)')
    parser.add_argument('-b', '--batch', dest='batch', action='store_true',
                        default=False,
                        help='Batch processing of input directory')
    parser.add_argument('--from', dest='from', action='store',
                        default='convert',
                        choices=PROCESSES,
                        help='Which command to start from')
    parser.add_argument('--action', dest='actions', action='append',
                        choices=PROCESSES,
                        help='What actions to do')
    parser.add_argument('--overwrite', dest='overwrite', action='store_true', default=False, 
                        help='Overwrite existing files')
    parser.add_argument('--novh', dest='novh', action='store_true', default=False, 
                        help='Remove _vh suffixes')
    parser.add_argument('--test', dest='test', action='store_true', default=False, 
                        help='Test pipeline commands without executing them')

    args = parser.parse_args()
    config = {}
    if args.overwrite:
        config['overwrite'] = True
    if args.novh:
        config['novh'] = True
    if args.actions:
        for action in args.actions:
            config[action] = True
    else:
        config = update_config(vars(args))

    global TEST_MODE
    TEST_MODE = args.test
    util.setCallTestMode(args.test)

    if os.path.isdir(args.input):
        if args.batch:
            process_scan_dir_batch(args.input, config)
        else:
            name = os.path.relpath(args.input, args.input + '/..')
            process_scan_dir(args.input, name, config)
    elif os.path.isfile(args.input):
        dirs = util.readlines(args.input)
        process_scan_dirs(dirs, config)
    else:
        print('Please specify directory or file as input')


if __name__ == "__main__":
    main()
