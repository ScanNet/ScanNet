#!/usr/bin/env python
#
# Recommended:
# Install virtualenv
# Create virtualenv
# pip install flask
# Run with ./monitor.py (or python monitor.py on Windows)

import argparse
import json
import os
import logging
import traceback
import requests
from threading import Lock
from flask import Flask, request, jsonify
from werkzeug import secure_filename

import util
import index

SCRIPT_DIR = util.getScriptPath()
INDEX_ALL_BIN = os.path.join(SCRIPT_DIR, 'index_scannet.sh')
CONVERT_H264_TO_MP4_BIN = os.path.join(SCRIPT_DIR, 'scripts', 'h264_to_mp4.sh')
CONVERT_H264_TO_THUMBNAIL_BIN = os.path.join(SCRIPT_DIR, 'scripts', 'h264_thumbnail.sh')

CMD_ARGS = []
if os.name == 'nt':
    GIT_BASH = 'C:\\Program Files\\Git\\bin\\bash.exe'
    CMD_ARGS = [GIT_BASH]

# where scan data is stored under as subdirs with unique ids
STAGING_FOLDER = os.path.join(SCRIPT_DIR, 'staging')

# informing our management UI of our updated state
WEBUI = 'http://localhost:3030'

# for locking indexing resources
INDEX_LOCK = Lock()

# for locking convert video lock
CONVERT_VIDEO_LOCK = Lock()

app = Flask(__name__)
app.config['STAGING_FOLDER'] = STAGING_FOLDER
app.config['indexfile'] = 'scan-net.csv'
app.config['source'] = 'scan'
app.config['datasets'] = 'ScanNet'

FORMAT = '%(asctime)-15s [%(levelname)s] %(message)s'
logging.basicConfig(format=FORMAT)
log = logging.getLogger('monitor')
log.setLevel(logging.INFO)


def post(url, data, log):
    log.info('Connecting to ' + url + ' ...')
    try:
        resp = requests.post(url, json=data)
        log.info('Connected to ' + url + ' successfully.')
        return {'status': 'ok', 'response': resp.text}
    except requests.exceptions.RequestException as e:
        log.warning('Error connecting to ' + url + ': ' + e.reason)
        return {'status': 'error', 'message': e.reason}


@app.before_request
def log_request():
    log.info('Got request: %s', request.path)


# Indexes scan
@app.route('/index/<dirname>')
def index_scan(dirname):
    dirname = secure_filename(dirname)
    path = os.path.join(app.config['STAGING_FOLDER'], dirname)

    # Indexing
    with INDEX_LOCK:
        indexfile = os.path.join(app.config['STAGING_FOLDER'], app.config['indexfile'])
        indexed = index.index({
            'input': path, 'output': indexfile, 'root': app.config['STAGING_FOLDER'],
            'single': True, 'append': True, 'checkCleaned': True,
            'source': app.config['source'], 'datasets': app.config['datasets'],
            'stages': app.config['stages'],
            'includeAll': True
        })
        if indexed:
            res = post(WEBUI + '/scans/populate?group=staging&replace=true', indexed.values(), log)
            if res.get('status') == 'ok':
                return res.get('response')
            else:
                resp = jsonify({"message": res.message})
                resp.status_code = 500
                return resp
        else:
            return 'Nothing to index'


@app.route('/index')
def index_all():
    with INDEX_LOCK:
        ret = util.call(CMD_ARGS + [INDEX_ALL_BIN], log, desc='index_all')
        if ret < 0:
            resp = jsonify({"message": 'Error indexing all scans'})
            resp.status_code = 500
            return resp
        else:
            return 'done'


# Converts the h264 to mp4 and thumbnails
@app.route('/convert-video/<dirname>')
def convert_video(dirname):
    dirname = secure_filename(dirname)
    path = os.path.join(app.config['STAGING_FOLDER'], dirname)

    # Convert
    with CONVERT_VIDEO_LOCK:
        ret1 = util.call(CMD_ARGS + [CONVERT_H264_TO_MP4_BIN, '--skip-done', path], log, desc='h264-to-mp4')
        ret2 = util.call(CMD_ARGS + [CONVERT_H264_TO_THUMBNAIL_BIN, '--skip-done', path], log, desc='h264-thumbnail')
        if ret1 < 0 or ret2 < 0:
            resp = jsonify({"message": 'Error converting h264 to mp4/thumbnail'})
            resp.status_code = 500
            return resp
        else:
            return 'done'


@app.route('/health')
def health():
    return 'ok'


def main():
    scriptpath = os.path.dirname(os.path.realpath(__file__))
    # Argument processing
    parser = argparse.ArgumentParser(description='Start monitor web service')
    parser.add_argument('--stages', dest='stagesFile', action='store',
                        default=os.path.join(scriptpath, 'config/scan_stages.json'),
                        help='File specifying scan stages')
    parser.add_argument('--port', dest='port', action='store',
                        default=5001,
                        help='Port number')
    args = parser.parse_args()

    if args.stagesFile:
        with open(args.stagesFile) as json_data:
            app.config['stages'] = json.load(json_data)

    app.run(host='0.0.0.0', port=args.port, debug=True)


if __name__ == "__main__": main()
