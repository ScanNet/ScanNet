#!/usr/bin/env bash
#
# Reduces image sizes as well as optimizes PNGs
# Prerequisites for resize:
# http://imagemagick.org/
#
# Prerequisites for optimization:
# https://pngquant.org/
# http://optipng.sourceforge.net/
# 
# Both pngquant and optipng must be on the PATH

OUT_SIZE=128x128
OPTIMIZE=true
VERBOSE=false
SKIPDONE=false
CONVERT=${CONVERT:-convert}
PNGQUANT=${PNGQUANT:-pngquant}
OPTIPNG=${OPTIPNG:-optipng}

function run {
    echo "Resizing to ${OUT_SIZE}..."
    for img in `find "${DIR}" -name "*.png" -not -name "*_thumb.png"`; do
        thumb=${img/\.png/_thumb\.png}
        if [[ ("$SKIPDONE" = true) && (-s ${thumb}) ]]; then
            # Skip this
            continue
        fi   
        if [ "$VERBOSE" = true ]; then
            echo "Create thumbnail: ${img} to ${thumb}"
        fi
        "${CONVERT}" "${img}" -resize ${OUT_SIZE} "${thumb}"
        if [ "$OPTIMIZE" = true ]; then
            "${PNGQUANT}" 256 --ext .png --force "${thumb}" && "${OPTIPNG}" -clobber "${thumb}" 
        fi
    done
    echo "Done."
}

while true; do
    case "$1" in
        -h)
            echo "Usage: $0 [-h] [-v] [--skip-done] [-d] <dir> [size]"
            echo "Resizes images and optionally reduces image size in a lossy manner"
            echo "[-h] prints this help message and quits"
            echo "[-v] verbose mode"
            echo "[--skip-done] skips images with existing thumbnails"
            echo "[-d] is optional and disables optimization. pngquant and optipng must be on the PATH"
            echo "[size] is optional and must be in XxY format: 128x128"
            echo "All parameters are positional"
            exit 1
            ;;
        -d)
            OPTIMIZE=false
            shift
            DIR="$1"
            shift
            break
            ;;
        -v)
            VERBOSE=true
            shift
            ;;
        --skip-done)
            SKIPDONE=true
            shift
            ;;
        *)
            DIR="$1"
            shift
            break
    esac
done

if [ -z $DIR ]
then
   echo "Please specify input directory"
   exit 1
fi

if [ -n "$1" ]; then
    OUT_SIZE="$1"
fi

run
