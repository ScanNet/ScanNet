#!/usr/bin/env python
#
# Compute times from process.log
# May need pip install pytimeparse

import argparse
import collections
import csv
import os
import logging
import re
import subprocess
import sys
import traceback

from datetime import timedelta
import pytimeparse

FORMAT = '%(asctime)-15s [%(levelname)s] %(message)s'
logging.basicConfig(format=FORMAT)
log = logging.getLogger('computeTimings')
log.setLevel(logging.INFO)


def getTotal(times):
    secs = 0
    for k, r in times.iteritems():
        secs += r.get('secs')
    return {'name': 'total', 'time': str(timedelta(seconds=secs)), 'secs': secs}


def getRecord(times, name, n=None):
    if n is not None:
        secs = 0
        for i in range(1, n+1):
            r = times.get('%s%d' % (name, i))
            if r is not None:
                secs += r.get('secs')
        return {'name': name, 'time': str(timedelta(seconds=secs)), 'secs': secs}
    else:
        return times.get(name)


def computeTimings(input):
    try:
        timings = subprocess.check_output("grep 'Time' \"%s\"" % input, shell=True)
        timings = [t.strip() for t in timings.splitlines() if len(t.strip()) > 0]
    except subprocess.CalledProcessError as e:
        if e.returncode != 1:
            log.warning('Error extracting timings from %s', input)
            traceback.print_exc()
        return None
    except:
        log.warning('Error extracting timings from %s', input)
        traceback.print_exc()
        return None

    timePattern = re.compile('.*Time=([0-9.:]+)\s+for\s+(.*)')
    times = collections.OrderedDict()
    for timing in timings:
        # print timing
        m = timePattern.match(timing)
        if m is not None:
            time = m.group(1)
            cmd = m.group(2)
            if cmd.startswith('cmd'):
                cmdname = None
            else:
                pieces = cmd.split(', ')
                cmdname = pieces[0]
                cmd = pieces[1]
            if cmdname is not None:
                times[cmdname] = {'name': cmdname, 'time': time, 'secs': pytimeparse.parse(time)}
        else:
            log.warn('Error extracting time from %s', timing)
    return times


def saveCsv(fieldnames, data, csvfile):
    writer = csv.DictWriter(csvfile, fieldnames=fieldnames, extrasaction='ignore')
    writer.writeheader()
    for k, v in data.iteritems():
        writer.writerow(v)


def computeAndOutputTimings(args):
    input = args.get('inputfile')
    times = computeTimings(input)
    if times is not None:
        fieldnames = ['name', 'time', 'secs']
        if args.get('output'):
            with open(args.get('output'), 'wb') as outfile:
                saveCsv(fieldnames, times, outfile)
        else:
            saveCsv(fieldnames, times, sys.stdout)
            sys.stdout.flush()


def main():
    scriptpath = os.path.dirname(os.path.realpath(__file__))
    # Argument processing
    parser = argparse.ArgumentParser(description='Compute timings for processing of a scan')
    parser.add_argument('input', help='Input directory or log')
    parser.add_argument('output', nargs='?')

    args = parser.parse_args()
    if os.path.isdir(args.input):
        args.inputfile = os.path.join(args.input, 'process.log')
    elif os.path.isfile(args.input):
        args.inputfile = args.input
    else:
        log.error('Not a directory or file: %s', args.input)
    computeAndOutputTimings(vars(args))


if __name__ == "__main__": main()
