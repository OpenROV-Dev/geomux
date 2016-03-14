# GeoMux

Not yet fit for ROV consumption. Requires proper lib files for ffmpeg, mxuvc, and mxcam; not included in repo.

### dev dependencies to build:
openrov-ffmpeg-lib
openrov-geocamera-libs


After camera is connected, kernel driver setup, and camera properly booted/initialized, build and run with:

## Developing

If you are running as root, you will need the `--unsafe-perm` flag for the npm commands.

If you want to develop on just the index.js file
```
git clone https://<username>@github.com/openrov/geomux.git
cd GeoMux
npm version <version of the precompiled library>
npm link
cd <test project>
npm link geomux
```
If you want the ability to compile the C++ extension as well:
```
apt-get install -y build-essential 
wget http://openrov-software-nightlies.s3-us-west-2.amazonaws.com/jessie/geocamera-libs/openrov-geocamera-libs_1.0.0-1~22.23ead83_armhf.deb
  dpkg -i openrov-geocamera-libs_1.0.0-1~22.23ead83_armhf.deb
  wget http://openrov-software-nightlies.s3-us-west-2.amazonaws.com/jessie/ffmpeg/openrov-ffmpeg-lib_2.9.0-1~12.ce855bf_armhf.deb
  dpkg -i openrov-ffmpeg-lib_2.9.0-1~12.ce855bf_armhf.deb  
npm install --build-from-source
```

Another command for rebuilding the c++ addon which skips the additional npm steps in the beginning is:

```bash
node-gyp rebuild --module_name="geomux" --module_path="./lib/binding/"
```
