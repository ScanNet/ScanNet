#!/usr/bin/env bash
#
# Simple script to index scannet staged and checked

BIN=`dirname $0`
SCANNET_DIR=${1:-E:/share/data/scan-net/}
WEBUI=http://localhost:3030

SCANS_DIR=${SCANNET_DIR}/scans
ANNS_DIR=${SCANNET_DIR}/annotations

# Index
$BIN/../index.py -i $SCANS_DIR/staging -o $SCANS_DIR/staging/scan-net.csv
$BIN/../index.py --all --format json -i $SCANS_DIR/staging -o $SCANS_DIR/staging/scan-net.all.json
$BIN/../index.py -i $SCANS_DIR/checked -o $SCANS_DIR/checked/scan-net.csv
$BIN/../index.py --format json -i $SCANS_DIR/checked -o $SCANS_DIR/checked/scan-net.all.json

# If there is annotation stats, merge them in
ANN_STATS=${ANNS_DIR}/scannet.anns.stats.csv
if [ -f ${ANN_STATS} ]; then
  echo "Combining stats from ${ANN_STATS}"
  cp $SCANS_DIR/checked/scan-net.csv $SCANS_DIR/checked/scan-net.nostats.csv
  $BIN/combine_stats.py --format csv -i $SCANS_DIR/checked/scan-net.nostats.csv -i ${ANN_STATS} $SCANS_DIR/checked/scan-net.csv
  cp $SCANS_DIR/checked/scan-net.all.json $SCANS_DIR/checked/scan-net.all.nostats.json
  $BIN/combine_stats.py --format json -i $SCANS_DIR/checked/scan-net.all.nostats.json -i ${ANN_STATS} $SCANS_DIR/checked/scan-net.all.json
fi


# Tell our webui about it
curl "$WEBUI/scans/populate?group=staging&replace=group"  -H 'Content-Type: application/json' --data-binary "@$SCANS_DIR/staging/scan-net.all.json"
curl "$WEBUI/scans/populate?group=checked&replace=group"  -H 'Content-Type: application/json' --data-binary "@$SCANS_DIR/checked/scan-net.all.json"
