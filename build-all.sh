#!/bin/bash

# exit when any command fails
set -e

#
# Create 'build' directory
#
rm -rf build
mkdir build
cd build

#
# Run cmake
#

cmake .. -DCMAKE_CXX_COMPILER=clang++  -DCMAKE_C_COMPILER=clang -DBUILD_CONFIG=mysql_release
make -j2


#
# Run test
#
# ctest --verbose
#ctest
