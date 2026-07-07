#!/bin/bash
set -e

mkdir -p build
mkdir -p build/shaders
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)
