#!/bin/bash
sudo apt-get update -y && \
sudo apt-get install --no-install-recommends -y libssl-dev zlib1g-dev libsodium-dev libopus-dev

rm -rf build/
mkdir build && cd build || exit
cmake -S ../ -B .
# Out of memory error when "$(nproc)" is used
make -j 4