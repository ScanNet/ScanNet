import hashlib
import os
import subprocess as subp
import re
import sys
import traceback
import datetime

from flask import jsonify
from timeit import default_timer as timer


DEFAULT_CALL_TEST_MODE = False


def setCallTestMode(v):
    global DEFAULT_CALL_TEST_MODE
    DEFAULT_CALL_TEST_MODE = v


def call(cmd, log, rundir='', env=None, desc=None, testMode=None):
    if testMode is None:
        testMode = DEFAULT_CALL_TEST_MODE
    if not cmd:
        log.warning('No command given')
        return 0
    if testMode:
        log.info('Running ' + str(cmd))
        return -1
    cwd = os.getcwd()
    res = -1
    try:
        start_time = timer()
        if rundir:
            os.chdir(rundir)
            log.info('Currently in ' + os.getcwd())
        log.info('Running ' + str(cmd))
        prog = subp.Popen(cmd, stdout=subp.PIPE, stderr=subp.PIPE, env=env)
        out, err = prog.communicate()
        if out:
            log.info(out)
        if err:
            log.error('Errors reported running ' + str(cmd))
            log.error(err)
        end_time = timer()
        delta_time = end_time - start_time
        desc_str = desc + ', ' if desc else ''
        desc_str = desc_str + 'cmd="' + str(cmd) + '"'
        log.info('Time=' + str(datetime.timedelta(seconds=delta_time)) + ' for ' + desc_str)
        res = prog.returncode
    except Exception as e:
        log.error(traceback.format_exc())
    os.chdir(cwd)
    return res


def is_non_zero_file(fpath):
    return True if os.path.isfile(fpath) and os.path.getsize(fpath) > 0 else False


def ensure_dir_exists(path):
    try:
        if not os.path.isdir(path):
            os.makedirs(path)
    except OSError:
        if not os.path.isdir(path):
            raise


def filesize(fpath):
    if os.path.isfile(fpath):
        return os.path.getsize(fpath)
    else:
        return 0


def getScriptPath():
    return os.path.dirname(os.path.realpath(sys.argv[0]))


def read_properties(fpath, log):
    # Read in txt as dictionary
    try:
        lines = filter(None, (line.rstrip() for line in open(fpath)))
        props = dict(line.strip().split('=', 2) for line in lines)
        # Strip whitespace (can also do so by using regex (re) to split)
        props = {k.strip(): v.strip() for k, v in props.iteritems()}
        return props
    except Exception as e:
        log.error('Error reading properties from ' + fpath)
        log.error(traceback.format_exc())
        return False


def list_files(dirname):
    # Get list of files in directory and their name, size, date
    files = os.listdir(dirname)
    fileinfos = []
    for file in files:
        st = os.stat(os.path.join(dirname, file))
        mtimestr = datetime.datetime.fromtimestamp(st.st_mtime).strftime('%Y-%m-%d %H:%M:%S.%f')[:-3]
        name = re.sub(r'^' + dirname + '/+', '', file)
        fileinfos.append({
            'name': name,
            'size': st.st_size,
            'modifiedAt': mtimestr,
            'modifiedAtMillis': long(round(st.st_mtime*1000))
        })
    return fileinfos


def lastModified(fileinfos):
    # Get the last modifiedAt string
    last = None
    for fileinfo in fileinfos:
        if last == None or fileinfo.get('modifiedAtMillis') > last.get('modifiedAtMillis'):
            last = fileinfo
    return last


def millisToIso(millis):
    # Return timestamp in ISO 8601
    mtimestr = datetime.datetime.fromtimestamp(millis/1000.0).strftime('%Y-%m-%dT%H:%M:%S')
    return mtimestr


def secsToIso(secs):
    # Return timestamp in ISO 8601
    mtimestr = datetime.datetime.fromtimestamp(secs).strftime('%Y-%m-%dT%H:%M:%S')
    return mtimestr


def checkLastModifiedNewer(dirname, timestamp):
    fileinfos = list_files(dirname)
    last = lastModified(fileinfos)
    if last is not None:
        lastMs = last.get('modifiedAtMillis')
        # print('last modified is ' + str(lastMs) + ', request timestamp is ' + str(timestamp))
        return lastMs > timestamp
    else:
        return None


# https://stackoverflow.com/questions/3431825/generating-a-md5-checksum-of-a-file
def md5(filename, blocksize=65536):
    hash = hashlib.md5()
    with open(filename, 'rb') as f:
        for chunk in iter(lambda: f.read(blocksize), b''):
            hash.update(chunk)
    return hash.hexdigest()


# http://stackoverflow.com/questions/1094841/reusable-library-to-get-human-readable-version-of-file-size
def naturalsize(num, suffix='B'):
    for unit in ['', 'K', 'M', 'G', 'T', 'P', 'E', 'Z']:
        if abs(num) < 1024.0:
            return "%3.1f%s%s" % (num, unit, suffix)
        num /= 1024.0
    return "%.1f%s%s" % (num, 'Yi', suffix)


# Read lines and returns as list (ignores empty lines)
def readlines(input):
    lines = []
    with open(input) as x:
        for line in x:
            line = line.strip()
            if len(line):
                lines.append(line)
    return lines


# Returns a recursive directory tree for path (relative to base_dir)
def make_tree(base_dir, path):
    tree = dict(name=os.path.basename(path), relative_name=path.replace(base_dir, 'received') , children=[])
    try: lst = os.listdir(path)
    except OSError:
        pass #ignore errors
    else:
        for name in lst:
            fn = os.path.join(path, name)
            if os.path.isdir(fn):
                tree['children'].append(make_tree(base_dir, fn))
            else:
                st = os.stat(fn)
                mtimestr = datetime.datetime.fromtimestamp(st.st_mtime).strftime('%Y-%m-%d %H:%M:%S.%f')[:-3]
                tree['children'].append(dict(name=name, fileSize=naturalsize(st.st_size), modifiedAt=mtimestr,
                    relative_name=os.path.join(path.replace(base_dir, 'received'), name)))
    return tree


# Return 200 OK message
def ret_ok(message = 'ok'):
    rv = {}
    rv['message'] = message
    ok_resp = jsonify(rv)
    ok_resp.status_code = 200
    return ok_resp


# Simple error class
class Error(Exception):
    def __init__(self, message='', status_code=500):
        Exception.__init__(self)
        self.message = message
        self.status_code = status_code

    def to_dict(self):
        rv = {}
        rv['message'] = self.message
        return rv

    def to_json(self):
        return jsonify(self.to_dict())
