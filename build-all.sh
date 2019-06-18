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

cmake .. -DROMZ_CODE_COVERAGE_ENABLED=ON
make -j2


#
# Run test
#
# ctest --verbose
ctest
