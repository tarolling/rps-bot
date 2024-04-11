#!/bin/sh

# Install packages
sudo apt-get update -y && \
sudo apt-get install --no-install-recommends -y libssl-dev zlib1g-dev libsodium-dev libopus-dev libspdlog-dev=1:1.5.0-1 libfmt-dev=6.1.2+ds-2 cmake pkg-config g++ gcc git make && \
sudo apt-get clean
# Symlink weirdness
ln -s libcrypto.so.3 libcrypto.so
ln -s libssl.so.3 libssl.so
sudo ldconfig
# Install DPP dependency
wget -O dpp.deb https://github.com/brainboxdotcc/DPP/releases/download/v10.0.29/libdpp-10.0.29-linux-x64.deb
sudo -s dpkg -i dpp.deb
rm dpp.deb

rm -rf build/
mkdir build && cd build || exit
cmake -S .. -B .
make -j "$(nproc)"