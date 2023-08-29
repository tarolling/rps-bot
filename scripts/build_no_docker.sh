#!/bin/bash
sudo apt-get update -y && \
sudo apt-get install --no-install-recommends -y libssl-dev zlib1g-dev libsodium-dev libopus-dev

sudo wget -O dpp.deb https://dl.dpp.dev/
sudo -s dpkg -i dpp.deb
rm dpp.deb

mkdir build && cd build || exit
cmake -S ../ -B .
make -j "$(nproc)"