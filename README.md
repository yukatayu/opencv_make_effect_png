## Setup

- CUDA
  - https://docs.nvidia.com/cuda/wsl-user-guide/index.html

```bash
sudo apt install nvidia-cuda-toolkit
ln -s $(which gcc-8) /usr/local/bin/gcc
ln -s $(which g++-8) /usr/local/bin/g++
```

## Build

```bash
mkdir -p build
rm -rf build/*

pushd build
cmake .. -DMAX_THREAD=8
make
popd
```


## Run

```bash
mkdir -p png
rm -f png/*.png
time build/main 30 1920 1080  # 30 fps, 1920x1080 px
```


### Convert

```bash
rm -f out.mov
ffmpeg -framerate 30 -i png/out_%06d.png -r 30 -pix_fmt argb -c:v qtrle out.mov
```
