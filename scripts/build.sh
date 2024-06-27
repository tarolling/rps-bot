#!/bin/sh

rm -rf build/
mkdir build && cd build || exit
cmake -S .. -B . -DCMAKE_BUILD_TYPE=Debug
make -j 4