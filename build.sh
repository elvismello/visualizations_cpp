#!/bin/bash

rm -rf build
mkdir -p build
cd build
/snap/bin/cmake --preset gcc14 -S ..
make -j$(nproc)