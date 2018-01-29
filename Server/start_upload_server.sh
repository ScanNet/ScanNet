#!/bin/sh
OLD_PATH="`pwd`"
MY_PATH="`dirname \"$0\"`"
cd ${MY_PATH}
#gunicorn -b 0.0.0.0:8000 -t 0 wsgi:app -w 2
gunicorn --paste config/upload.ini
cd ${OLD_PATH}
