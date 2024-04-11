#!/bin/sh

# Install packages
sudo apt-get update -y && \
sudo apt-get install --no-install-recommends -y libssl-dev zlib1g-dev libsodium-dev libopus-dev libspdlog-dev=1.13.0 libfmt-dev=8.1.1 cmake pkg-config g++ gcc git make && \
sudo apt-get clean
# Install DPP dependency
wget -O dpp.deb https://dl.dpp.dev/
sudo -s dpkg -i dpp.deb
rm dpp.deb

rm -rf build/
mkdir build && cd build || exit
cmake -S .. -B .
make -j "$(nproc)"