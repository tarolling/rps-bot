#!/bin/bash
sudo apt-get update -y && \
sudo apt-get install --no-install-recommends -y libssl-dev zlib1g-dev libsodium-dev libopus-dev

sudo -s dpkg -i dpp.deb

mkdir build && cd build || exit
cmake -S ../ -B .
make -j "$(nproc)"