rm -rf build/
mkdir build && cd build || exit
cmake -S ../ -B .
make -j "$(nproc)"