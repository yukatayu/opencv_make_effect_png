
## 構成
- png/ - png画像の連番が出力されます。
- render/〈render_name〉/main.hpp - 個別のレンダラ
  - 実装すべきインターフェースは `protocol.hpp` 及びサンプルを参照

## Setup

- CUDA
  - https://docs.nvidia.com/cuda/wsl-user-guide/index.html

```bash
sudo apt install -y build-essential g++-8 libopencv-dev ffmpeg nvidia-cuda-toolkit

ln -s $(which gcc-8) /usr/local/bin/gcc
ln -s $(which g++-8) /usr/local/bin/g++
```

## Build (CPU)

```bash
mkdir -p build
rm -rf build/*

pushd build
cmake .. -DBUILD_CPU=ON -DRENDERER='square_transition' -DMAX_THREAD=8
make
popd
```

## Build (GPU)

```bash
mkdir -p build
rm -rf build/*

pushd build
cmake .. -DBUILD_GPU=ON -DRENDERER='square_transition'
make
popd
```

## Run

```bash
mkdir -p png
rm -f png/*.png
time build/main     30 1920 1080  # CPU / 30 fps, 1920x1080 px
time build/main_gpu 30 1920 1080  # GPU / 30 fps, 1920x1080 px
```


## Convert

#### png -> mov

```bash
rm -f out.mov
ffmpeg -framerate 30 -i png/out_%06d.png -r 30 -pix_fmt argb -c:v qtrle out.mov
```

#### 備考: mov -> webm

ビットレート = 2048K の場合，

```bash
ffmpeg -i out.mov -c:v libvpx-vp9 -pass 1 -b:v 2048K -threads 8 -speed 4 -tile-columns 6 -frame-parallel 1 -an -f webm -y /dev/null
ffmpeg -i out.mov -c:v libvpx-vp9 -pass 2 -b:v 2048K -threads 8 -speed 1 -tile-columns 6 -frame-parallel 1 -auto-alt-ref 1 -lag-in-frames 25 -c:a libopus -b:a 64k -f webm -y out.webm
rm -f ffmpeg2pass-0.log
```

#### 備考: webm -> mov

```bash
./ffmpeg -c:v libvpx-vp9 -i out.webm -pix_fmt argb -c:v qtrle out.mov
```
