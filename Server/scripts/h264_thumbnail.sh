#!/usr/bin/env bash
#
# Generates thumbnails for h264 raw video streams

OUT_WIDTH=200
VERBOSE=false
SKIPDONE=false
FFMPEG=${FFMPEG:-ffmpeg}
INPUT_PATTERN="*.h264"

function run {
    for file in `find "${DIR}" -name "$INPUT_PATTERN"`; do
        thumb=${file%.*}_thumb\.jpg
        if [[ ("$SKIPDONE" = true) && (-s ${thumb}) ]]; then
            # Skip this
            continue
        fi
        if [ "$VERBOSE" = true ]; then
            echo "Create thumbnail: ${file} to ${thumb}"
        fi
        "${FFMPEG}" -y -i "${file}" -vf "scale=${OUT_WIDTH}:-1" -frames:v 1 "${thumb}"
    done
    echo "Done."
}

while true; do
    case "$1" in
        -h)
            echo "Usage: $0 [-h] [-i pattern] [-v] [--skip-done] <dir> [width]"
            echo "Creates thumbnail images for .h264 raw streams"
            echo "[-h] prints this help message and quits"
            echo "[-i] specified input file pattern"
            echo "[-v] verbose mode"
            echo "[--skip-done] skips images with existing thumbnails"
            echo "[width] is optional and must be width in pixels"
            echo "All parameters are positional"
            exit 1
            ;;
        -v)
            VERBOSE=true
            shift
            ;;
        -i)
            shift
            INPUT_PATTERN=$1
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
    OUT_WIDTH="$1"
fi

run
