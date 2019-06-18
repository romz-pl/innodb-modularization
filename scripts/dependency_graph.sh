#!/bin/bash

if [ $# -lt 1 ]; then
    echo "Usage: <path-to-src-directory> <optional-cmake-flags>"
    exit 1
fi

#
# The first argument: path to source directory.
#
SOURCE_DIR=$1

#
# The second argument: Cmake flags (could be empty)
#
CMAKE_FLAGS=$2

#
# GraphVis dot file with libraries with prefix "romz-"
#
DOT_FILE_WITH_PREFIX=long_names.dot

#
# PNG file with libraries with prefix "romz-"
#
PNG_FILE_WITH_PREFIX=${DOT_FILE_WITH_PREFIX}.png

#
# Generate DOT file forma CMakeFile.txt
#
cmake ${CMAKE_FLAGS} --graphviz=${DOT_FILE_WITH_PREFIX} ${SOURCE_DIR} || exit 1

#
# Generate PNG file with prefix "romz-"
#
dot ${DOT_FILE_WITH_PREFIX} -T png -o ${PNG_FILE_WITH_PREFIX} || exit 1


#
# GraphVis dot file with libraries without prefix "romz-"
#
DOT_FILE_WITHOUT_PREFIX=short_names.dot

#
# PNG file with libraries without prefix "romz-"
#
PNG_FILE_WITHOUT_PREFIX=${DOT_FILE_WITHOUT_PREFIX}.png

#
# Remove prefix "borg-"
#
prefix='romz-'
cat ${DOT_FILE_WITH_PREFIX} | sed -e "s/$prefix//" > ${DOT_FILE_WITHOUT_PREFIX}

#
# Generate PNG file without prefix "romz-"
#
dot ${DOT_FILE_WITHOUT_PREFIX} -T png -o ${PNG_FILE_WITHOUT_PREFIX} || exit 1

#
# Create output directory for PNG files
#
OUT_DIR=png_files
mkdir -p ${OUT_DIR} || exit 1

#
# Move PNG files to output directory
#
mv ${PNG_FILE_WITH_PREFIX} ${OUT_DIR} || exit 1
mv ${PNG_FILE_WITHOUT_PREFIX} ${OUT_DIR} || exit 1

RED='\033[0;31m'
NC='\033[0m' # No Color

echo "Dependecy graphs have been successfully generated."
echo -e "${RED}  See file ${PWD}/${OUT_DIR}/${PNG_FILE_WITH_PREFIX}  ${NC}"
echo "   or"
echo -e "${RED}  See file ${PWD}/${OUT_DIR}/${PNG_FILE_WITHOUT_PREFIX}  ${NC}"
