#!/bin/bash

# sudo apt update && sudo apt upgrade -y
# sudo apt install -y build-essential gcc libopencv-dev
# memo: libgtkの検討

mkdir -p build
rm -rf build/*

pushd build
cmake ..
make
popd

mkdir -p png
rm -f png/*.png
build/main

ffmpeg -framerate 30 -i png/out_%06d.png -pix_fmt argb -c:v qtrle -r 30 out.mov
# ffmpeg -framerate 30 -i png/out_%06d.png -c:v qtrle -r 30 out.mov
