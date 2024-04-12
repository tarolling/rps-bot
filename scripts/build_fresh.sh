#!/bin/sh

# Install packages
sudo apt-get update -y
sudo apt-get install -y sudo gcc g++ build-essential git cmake libssl-dev libspdlog-dev=1:1.5.0-1 zlib1g-dev curl wget libpcre3-dev libsodium23 libopus0 nlohmann-json3-dev libfmt-dev=6.1.2+ds-2 libmysqlclient-dev
sudo apt-get clean
rm -rf /var/lib/apt/lists/*
sudo apt-get install --no-install-recommends -y sudo gcc g++ build-essential git cmake libssl-dev zlib1g-dev libsodium-dev libopus-dev libspdlog-dev=1:1.5.0-1 libfmt-dev=6.1.2+ds-2 lohmann-json3-dev pkg-config make && \
sudo apt-get clean
# Symlink weirdness
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