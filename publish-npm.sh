#!/bin/bash
set -e
export PATH=`pwd`/node_modules/.bin/:$PATH
set +x
export node_pre_gyp_accessKeyId=${s3Key}
export node_pre_gyp_secretAccessKey=${s3Secret}
set -x

./node_modules/.bin/node-pre-gyp publish
