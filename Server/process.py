#!/usr/bin/env python
#
# Recommended:
# Install virtualenv
# Create virtualenv
# pip install flask
# Run with ./process.py (or python process.py on Windows)

import os
import logging
import urllib2

import util
import index
import scan_processor as sp

from threading import Lock
from flask import Flask, request, jsonify
from werkzeug import secure_filename
import config as cfg

# for locking GPU resources
GPU_LOCK = Lock()

app = Flask(__name__)

INDEX_URL = os.path.join(cfg.DATA_SERVER, '/scans/index/')  # indexing endpoint

FORMAT = '%(asctime)-15s [%(levelname)s] %(message)s'
logging.basicConfig(format=FORMAT)
log = logging.getLogger('processor')
log.setLevel(logging.INFO)


def trigger_indexing(basename, log):
    index_url = INDEX_URL + basename
    log.info('Indexing ' + basename + ' at ' + index_url + ' ...')
    try:
        response = urllib2.urlopen(index_url)
        html = response.read()
        log.info('Index ' + basename + ' successfully.')
    except urllib2.URLError as e:
        log.warning('Error indexing ' + index_url + ': ' + e.reason)


@app.route('/process/<dirname>')
def process_scan_dir(dirname):
    dirname = secure_filename(dirname)
    path = os.path.join(cfg.STAGING_FOLDER_LOCAL, dirname)
    args = request.args
    if 'from' not in args and 'actions' not in args:
        config = {'all': True}
    else:
        config = {'from': args.get('from'), 'actions': args.get('actions')}
    config['overwrite'] = args.get('overwrite').lower() in ['true', '1', 'yes'] if 'overwrite' in args else False
    # print config
    sp.update_config(config)

    if config.get('overwrite'):
        # Check timestamp of request (only trigger if the last update timestamp before overwrite)
        # Prevents duplicates request
        timestamp = args.get('timestamp')
        if timestamp is None:
            resp = jsonify({'message': 'Please provide timestamp with request'})
            resp.status_code = 400
            return resp
        # Request timestamp should be in milliseconds UTC
        timestamp = long(timestamp)
        newer = util.checkLastModifiedNewer(path, timestamp)
        if newer:
            resp = jsonify({'message': 'Scan '+ dirname + ' modified after request issued, please resubmit request'})
            resp.status_code = 400
            return resp

    with GPU_LOCK:
       processed = sp.process_scan_dir(path, dirname, config)

    trigger_indexing(dirname, log)

    return processed


app.run(host='0.0.0.0', port=5000, debug=True)
