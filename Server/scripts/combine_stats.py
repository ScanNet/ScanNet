#!/usr/bin/env python
#
# Merges stats with index

import argparse
import collections
import csv
import json
import os
import logging
import sys


FORMAT = '%(asctime)-15s [%(levelname)s] %(message)s'
logging.basicConfig(format=FORMAT)
log = logging.getLogger('mergeStats')
log.setLevel(logging.INFO)

def loadJson(infile):
    rows = json.load(infile)
    if type(rows) is list:
        byId = {}
        for row in rows:
            byId[row['id']] = row
        rows = byId
    fieldnames = set()
    for id, row in rows.iteritems():
        fieldnames.update(row.keys())
    return {'fieldnames': list(fieldnames), 'rows': rows}


def saveJson(data, out):
    json.dump(data, out, indent=1, separators=(',', ': '))


def loadCsv(infile):
    reader = csv.DictReader(infile)
    rows = {}
    for row in reader:
        rows[row['id']] = row
    return {'fieldnames': list(reader.fieldnames), 'rows': rows}


def saveCsv(fieldnames, data, csvfile):
    writer = csv.DictWriter(csvfile, fieldnames=fieldnames, extrasaction='ignore')
    writer.writeheader()
    for k,v in data.iteritems():
        writer.writerow(v)


def loadFile(filename):
    with open(filename, 'r') as infile:
        if filename.endswith('.csv'):
            return loadCsv(infile)
        elif filename.endswith('.json'):
            return loadJson(infile)
        else:
            log.warning('Unknown file type ' + filename)


def saveFile(format, fieldnames, entries, outfile):
    if format == 'csv':
        saveCsv(fieldnames, entries, outfile)
    elif format == 'json':
        saveJson(entries, outfile)
    else:
        log.warning('Unknown file format ' + format)


def combine(args):
    inputs = args.get('input')
    if len(inputs) == 0:
        log.error('No inputs specified!!!')
        return
    data = loadFile(inputs[0])
    fieldnames = data.get('fieldnames')
    entries = data.get('rows')

    # Read read of the data and merge in by id
    ignored = collections.Counter()
    for filename in inputs[1:]:
        data = loadFile(filename)
        newfieldnames = [f for f in data.get('fieldnames') if f not in fieldnames]
        fieldnames.extend(newfieldnames)
        for id, datum in data.get('rows').iteritems():
            if id in entries:
                # merge in
                entries[id].update(datum)
            else:
                ignored.update([id])

    if args.get('output'):
        with open(args.get('output'), 'wb') as outfile:
            saveFile(args.get('format'), fieldnames, entries, outfile)
    else:
        saveFile(args.get('format'), fieldnames, entries, sys.stdout)
        sys.stdout.flush()


def main():
    scriptpath = os.path.dirname(os.path.realpath(__file__))
    # Argument processing
    parser = argparse.ArgumentParser(description='Merge index file with stats!')
    parser.add_argument('-i','--input', dest='input', action='append',
        help='Input files')
    parser.add_argument('--format', dest='format', action='store',
                        default='csv', choices=['csv', 'json'],
                        help='Format to use for output')
    parser.add_argument('output', nargs='?')

    args = parser.parse_args()
    combine(vars(args))


if __name__ == "__main__": main()