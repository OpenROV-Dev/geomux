#!/bin/bash
set -ex
apt-get update && apt-get install -y wget build-essential
ARCH=`uname -m`
if [ ${ARCH} = "armv7l" ]
then
  ARCH="armhf"
fi

#This is temporary until we push the deb files to our registry
set +e
PKG_OK=$(dpkg-query -W --showformat='${Status}\n' 'openrov-geocamera-libs_1.0.0-1~14*'|grep "install ok installed")
set -e
echo Checking for openrov-geocamera-libs: $PKG_OK
if [ "" == "$PKG_OK" ]; then
  wget http://openrov-software-nightlies.s3-us-west-2.amazonaws.com/jessie/geocamera-libs/openrov-geocamera-libs_1.0.0-1~25.d3bae9a_armhf.deb
  dpkg -i openrov-geocamera-libs_1.0.0-1~25.d3bae9a_armhf.deb
fi

set +e
PKG_OK=$(dpkg-query -W --showformat='${Status}\n' 'openrov-ffmpeg-lib_2.9.0-1~12*'|grep "install ok installed")
set -e
echo Checking for openrov-ffmpeg-lib: $PKG_OK
if [ "" == "$PKG_OK" ]; then
  wget http://openrov-software-nightlies.s3-us-west-2.amazonaws.com/jessie/ffmpeg/openrov-ffmpeg-lib_2.9.0-1~12.ce855bf_armhf.deb
  dpkg -i openrov-ffmpeg-lib_2.9.0-1~12.ce855bf_armhf.deb
fi

#--unsafe-perm for jenkins build
# TODO: Pull the version number from the package.json instead of the hard code
npm install --build-from-source --unsafe-perm
./node_modules/.bin/node-pre-gyp package
