#!/bin/bash

# sudo apt update && sudo apt upgrade -y
# sudo apt install -y build-essential gcc libopencv-dev
# memo: libgtkの検討

g++ -std=c++17 -I /usr/include/opencv4 -L /usr/local/lib -O2 -g -fsanitize=undefined,address Main.cpp -o _bin && echo '-- start --' && ./_bin
