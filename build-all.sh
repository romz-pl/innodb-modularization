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

cmake ..
make -j2


#
# Run test
#
# ctest --verbose
ctest
