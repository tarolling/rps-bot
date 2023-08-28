#!/bin/bash
wget wget -O dpp.deb https://dl.dpp.dev/
dpkg -i dpp.deb
rm dpp.deb

sudo apt-get install 
mkdir build && cd build
cmake -S ../ -B .
make -j $(nproc)