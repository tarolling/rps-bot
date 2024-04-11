#!/bin/sh

# Install packages
sudo apt-get update -y && \
sudo apt-get install --no-install-recommends -y libssl-dev zlib1g-dev libsodium-dev libopus-dev libspdlog-dev libfmt-dev cmake pkg-config g++ gcc git make && \
sudo apt-get clean
# Install DPP dependency
wget -O dpp.deb https://dl.dpp.dev/
sudo -s dpkg -i dpp.deb
rm dpp.deb

rm -rf build/
mkdir build && cd build || exit
cmake -S ../ -B .
make -j "$(nproc)"