#!/bin/bash
set -e

sudo apt update
sudo apt upgrade -y

sudo apt install -y build-essential g++-8 libopencv-dev ffmpeg
