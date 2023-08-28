#!/bin/bash
sudo apt-get update -y && \
apt-get install --no-install-recommends -y libssl-dev zlib1g-dev libsodium-dev libopus-dev

sudo wget wget -O dpp.deb https://dl.dpp.dev/
dpkg -i dpp.deb
rm dpp.deb

mkdir build && cd build
cmake -S ../ -B .
make -j $(nproc)