#!/bin/sh

# Install packages
sudo apt-get update -y && \
sudo apt-get install --no-install-recommends -y sudo gcc g++ build-essential git cmake libssl-dev libspdlog-dev=1:1.5.0-1 zlib1g-dev curl wget libpcre3-dev libsodium23 libopus0 nlohmann-json3-dev libfmt-dev=6.1.2+ds-2 libmysqlclient-dev && \
sudo apt-get clean && \
sudo rm -rf /var/lib/apt/lists/*
# Symlink weirdness
sudo ldconfig /x86_64-linux-gnu/


# Install DPP dependency
wget -O dpp.deb https://github.com/brainboxdotcc/DPP/releases/download/v10.0.30/libdpp-10.0.30-linux-x64.deb
sudo -s dpkg -i dpp.deb
rm dpp.deb

rm -rf build/
mkdir build && cd build || exit
cmake -S .. -B . -DDPP_CORO=on
make -j "$(nproc)"