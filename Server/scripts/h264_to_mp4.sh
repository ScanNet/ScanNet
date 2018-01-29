#!/usr/bin/env bash
#
# Converts h264 raw video streams to compressed mp4

OUT_WIDTH=324
VERBOSE=false
SKIPDONE=false
FFMPEG=${FFMPEG:-ffmpeg}

function run {
    for h264 in `find "${DIR}" -name "*.h264"`; do
        mp4=${h264/\.h264/\.mp4}
        if [[ ("$SKIPDONE" = true) && (-s ${mp4}) ]]; then
            # Skip this
            continue
        fi
        if [ "$VERBOSE" = true ]; then
            echo "Create mp4: ${h264} to ${mp4}"
        fi
        "${FFMPEG}" -y -i "${h264}" -vf "scale=${OUT_WIDTH}:-1" -pix_fmt yuv420p -r 25 -movflags faststart "${mp4}"
    done
    echo "Done."
}

while true; do
    case "$1" in
        -h)
            echo "Usage: $0 [-h] [-v] [--skip-done] <dir> [width]"
            echo "Converts.h264 raw stream to lower bitrate .mp4"
            echo "[-h] prints this help message and quits"
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
