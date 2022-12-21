#!/bin/bash
set -e

# sudo apt update && sudo apt upgrade -y
# sudo apt install -y build-essential gcc libopencv-dev

mkdir -p build
rm -rf build/*

pushd build
cmake ..
make
popd

mkdir -p png
rm -f png/*.png
build/main 30 2 1920 1080  # 30 fps, 2 sec, 1920x1080 px

rm -f out.mov
ffmpeg -framerate 30 -i png/out_%06d.png -r 30 -pix_fmt argb -c:v qtrle out.mov
# ffmpeg -framerate 30 -i png/out_%06d.png -r 30 -y test.mp4

echo "done."