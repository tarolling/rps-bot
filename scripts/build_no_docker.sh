#!/bin/bash
wget wget -O dpp.deb https://dl.dpp.dev/
sudo dpkg -i dpp.deb
rm dpp.deb

mkdir build && cd build
cmake -S ../ -B .
make -j $(nproc)