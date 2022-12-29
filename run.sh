#!/bin/bash
set -e
cd `dirname $0`

mkdir -p build
rm -rf build/*

pushd build
cmake .. -DMAX_THREAD=8 -DRENDERER='square_transition'
make
popd

mkdir -p png
rm -f png/*.png
# time build/main 30 3840 2160  # 30 fps, 3840x2160 px
# time build/main 30 1920 1080  # 30 fps, 1920x1080 px
time build/main_gpu 30 1920 1080  # 30 fps, 1920x1080 px
# build/main 30 480 270  # 30 fps, 480x270 px

rm -f out.mov
ffmpeg -framerate 30 -i png/out_%06d.png -r 30 -pix_fmt argb -c:v qtrle out.mov

echo "done."
