#!/usr/bin/env python 
#
# Install dependencies using install_deps.sh
# Run using start_upload_server.sh
# 
# To mimic an upload from the iPad app use this curl command:
# curl -v -X PUT -H "FILE_NAME: test.h264" --data-binary "@<path_to_some_file>" -H "Content-Type: application/ipad_scanner_data" "http://localhost:8000/upload"
# curl -v "localhost:8000/verify?filename=<base_file_name>&checksum=<check_sum>"

import os
import shutil
import traceback
import logging
import urllib2
import threading
import util

from flask import Flask, request, render_template_string, send_from_directory
from util import Error
from werkzeug import secure_filename
import config as cfg

app = Flask(__name__)

INDEX_URL = cfg.DATA_SERVER + '/scans/index/'
CONVERT_VIDEO_URL = cfg.DATA_SERVER + '/scans/monitor/convert-video/'
ALLOWED_EXTENSIONS = set(['h264', 'depth', 'imu', 'txt', 'camera'])
SCAN_PROCESS_TRIGGER_URL = cfg.DATA_SERVER + '/scans/process/'  # WebUI process url is nonblocking (it places it on a queue)
SCAN_PROCESS_COMPLETE_STATUS = 'Queued'


log = logging.getLogger('scanner-ipad-server')


@app.errorhandler(util.Error)
def handle_error(error):
    response = error.to_json()
    response.status_code = error.status_code
    return response


@app.before_request
def log_request_headers():
    log.info('Got request: %s', request.headers)


@app.after_request
def log_request_response(response):
    log.info('Got response: %s', response)
    return response


def allowed_file(filename):
    return '.' in filename and \
            filename.rsplit('.', 1)[1] in ALLOWED_EXTENSIONS


# returns whether scan completely uploaded to dir (assumes only scan files are being received)
def scan_done_uploading(dir):
    return len(os.listdir(dir)) >= len(ALLOWED_EXTENSIONS)  # TODO: improve check


# trigger indexing of scan
def trigger_indexing(basename, log):
    index_url = INDEX_URL + basename
    log.info('Indexing ' + basename + ' at ' + index_url + ' ...')
    try:
        response = urllib2.urlopen(index_url)
        html = response.read()
        log.info('Index ' + basename + ' successfully.')
    except urllib2.URLError as e:
        log.warning('Error indexing ' + index_url + ': ' + str(e.reason))


# trigger video conversion of scan
def trigger_video_conversion(basename, log):
    convert_video_url = CONVERT_VIDEO_URL + basename
    log.info('Converting video ' + basename + ' at ' + convert_video_url + ' ...')
    try:
        response = urllib2.urlopen(convert_video_url)
        html = response.read()
        log.info('Convert video' + basename + ' successfully.')
    except urllib2.URLError as e:
        log.warning('Error converting video ' + convert_video_url + ': ' + str(e.reason))


# scan uploaed do some basic stuff with it
def preprocess(basename, log):
    trigger_video_conversion(basename, log)
    trigger_indexing(basename, log)


# calls processor server on given scan basename
def trigger_processing(basename, log):
    process_url = SCAN_PROCESS_TRIGGER_URL + basename
    log.info('Calling scan process script for ' + basename + ' at ' + process_url + ' ...')
    try:
        response = urllib2.urlopen(process_url)
        html = response.read()
        log.info(SCAN_PROCESS_COMPLETE_STATUS + ' ' + basename + ' successfully.')
    except urllib2.URLError as e:
        log.warning('Error calling scan process for ' + process_url + ': ' + e.reason)


# Receives file from request (consuming input)
# Writes file to output if specifed (otherwise, discards it)
def receive_file(request, filename, output=None):
    content_length = request.environ.get('CONTENT_LENGTH', 0)
    #TODO DEBUG STARTS HERE
    if 'Content-Range' in request.headers:
        # extract starting byte from Content-Range header string
        range_str = request.headers['Content-Range']
        start_bytes = int(range_str.split(' ')[1].split('-')[0])
        log.exception('Receiving %s: PARTIAL FILE RECEIVED: %s', filename, range_str)
    if not content_length:
        content_length = 0
    else:
        content_length = int(content_length)
    content_read = 0
    chunk_size = 4096*4
    stream = request.environ['wsgi.input']
    while True:
        if(content_read % (chunk_size * 1000) == 0):
            log.info("Receiving %s: Uploaded count: %d, \t Percent: %.2f", 
                filename, content_read, 100 * float(content_read) / content_length )
        try:
            chunk = stream.read(chunk_size)
        except Exception as e:
            log.exception('Receiving %s: Exception: %s while reading input. Aborting...', 
                filename, str(e))
            raise Error(message='Unexpected error while receiving', status_code=500)
        if len(chunk) == 0:
            break
        content_read += len(chunk)
        if output is not None:
            output.write(chunk)

    log.info("Receiving %s: Uploaded count: %d, \t Percent: %.2f", 
             filename, content_read, 100 * float(content_read) / content_length)
    if output is None:
        log.info('Discarding received file ' + filename)

    if content_read != content_length:
        log.error('Receiving %s: Expected length %d, received length %d. Aborting...', 
                  filename, content_length, content_read)
        raise Error(message='Unexpected error while receiving', status_code=400)


# Temporarily accept both PUT and POST.
# TODO: Remove POST. PUT is more appropriate
@app.route('/upload', methods=['PUT', 'POST'])
def upload_file():
    try:
        filename = request.headers.get('FILE_NAME')
        if 'process' in request.args:
            auto_process_scan = request.args.get('process').lower() in ['true', '1']
        else:
            auto_process_scan = cfg.AUTOPROCESS
        log.info('Receiving %s, autoprocess=%s', filename, auto_process_scan)

        if allowed_file(filename):
            filename = secure_filename(filename)
            # determine final staging path for file and check if the file already exists
            basename = os.path.splitext(filename)[0]
            stagingdir = os.path.join(cfg.STAGING_FOLDER, basename)
            stagingpath = os.path.join(stagingdir, filename)
            if os.path.exists(stagingpath):
                log.info('File already exists on server: %s', stagingpath)
                receive_file(request, filename)
                return util.ret_ok('File already exists on server')
            # temp location to receive stream
            tmppath = os.path.join(cfg.TEMP_FOLDER, filename)
            with open(tmppath, 'wb') as f:
                receive_file(request, filename, f)

            # move to staging area dir and return
            util.ensure_dir_exists(stagingdir)
            shutil.move(tmppath, stagingpath)  # TODO: check if move succeeded and log error if not
            log.info('Staged ' + filename + ' to ' + stagingdir)
            # If uploading is complete try to trigger processing
            if scan_done_uploading(stagingdir):
                log.info('Scan done uploading to ' + stagingdir)
                if auto_process_scan:
                    #NOTE: Comment out lines below to disable automated scan processing trigger
                    indexThread = threading.Thread(target=preprocess, args=(basename, log))
                    indexThread.start()
                    processThread = threading.Thread(target=trigger_processing, args=(basename, log))
                    processThread.start()
            return util.ret_ok()
        else:
            log.error('File type not allowed: ' + filename)
            log.error(request)
            raise Error(message=('File type not allowed: ' + filename), status_code=415)
    except Exception as e:
      log.error(traceback.format_exc())
      #raise Error(message=('Unknown exception encountered %s' % str(e)), status_code=500)
      raise e


@app.route('/received', methods=['GET'])
@app.route('/received/<path:filename>', methods=['GET'])
def get_file(filename=None):
    base_dir = cfg.STAGING_FOLDER
    path = base_dir
    if filename:
        full_path = os.path.join(path, filename)
        if not os.path.isdir(full_path):
            return send_from_directory(path, filename)
        else:
            path = full_path

    tmpl = '''
<!doctype html>
<title>Path: {{ tree.name }}</title>
<h1>{{ tree.name }}</h1>
<table cellpadding="10">
    <tr><th>Name</th><th>Last Modified</th><th>Size</th></tr>
{%- for item in tree.children recursive %}
    <tr>
      <td><a href="/{{ item.relative_name }}">{{ item.name }}</a></td>
      <td>{{ item.modifiedAt }}</td>
      <td>{{ item.fileSize }}</td>
    </tr>
{%- endfor %}
</table>
    '''
    return render_template_string(tmpl, tree=util.make_tree(base_dir, path))


@app.route('/verify', methods=['GET'])
def verify_file():
    filename = request.args.get('filename')
    checksum = request.args.get('checksum')
    filename = secure_filename(filename)
    basename = os.path.splitext(filename)[0]
    stagingdir = os.path.join(cfg.STAGING_FOLDER, basename)
    stagingpath = os.path.join(stagingdir, filename)

    if not os.path.exists(stagingpath):
        log.error('File %s does not exist', stagingpath)
        raise Error(message=('File %s does not exist' % stagingpath), status_code=404)

    calculated_checksum = util.md5(stagingpath)

    valid = calculated_checksum == checksum
    if valid:
        log.info('File %s successfully verified', filename)
        return util.ret_ok()
    else:
        log.error('File %s: hash mismatch. Given: %s, calculated: %s',
                  filename,
                  checksum,
                  calculated_checksum)
        raise Error(message=('File hash mismatch. Given: %s, calculated: %s' % (checksum, calculated_checksum)),
                    status_code=400)


@app.route('/process/<scanid>', methods=['GET'])
def process_scan(scanid=None):
    processThread = threading.Thread(target=trigger_processing, args=(scanid, log))
    processThread.start()
    return util.ret_ok()


def get_app(*args, **kwargs):
    return app
