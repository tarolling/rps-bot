#!/bin/sh

rm -rf build/
mkdir build && cd build || exit
cmake -S .. -B . -DCMAKE_BUILD_TYPE=Release
make -j 4