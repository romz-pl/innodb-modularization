#!/bin/bash

if [ $# -lt 1 ]; then
    echo "Usage: <absolute-path-to-borg-src-directory> <optional-cmake-flags>"
    exit 1
fi

set -e


# Absolute path to borg source directory.
BORG_SOURCE_DIR=$1
if [[ "${BORG_SOURCE_DIR}" != /* ]]; then
    echo "Error: first argument has to be absolute path to borg directory."
    exit 1
fi

# Cmake flags: could be empty
CMAKE_FLAGS=$2

# GraphVis dot file with libraries
PREFIX=graph.dot

# Generate DOT file forma CMakeFile.txt
cmake ${CMAKE_FLAGS} --graphviz=${PREFIX} ${BORG_SOURCE_DIR}


# Create output directory for PNG files
OUT_DIR=png_files
mkdir -p ${OUT_DIR}

dot ${PREFIX} -T png -o ${OUT_DIR}/all.png
dot ${PREFIX}.innobase -T png -o ${OUT_DIR}/innobase.png


RED='\033[0;31m'
NC='\033[0m' # No Color

echo "Dependecy graphs have been successfully generated."
echo -e "${RED}  See directory ${PWD}/${OUT_DIR}  ${NC}"
