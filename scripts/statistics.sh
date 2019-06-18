#!/bin/bash

#
# Returns total number of lines in *.cpp and *.h files located in given directory
#

if [ $# -lt 1 ]; then
    echo "Usage: <path-to-src-directory>"
    exit 1
fi

SOURCE_DIR=$1

echo "Total number of *.cpp files:"
find ${SOURCE_DIR} -name *.cpp | wc -l

echo -e "\nTotal number of lines in *.cpp files:"
wc -l $(find ${SOURCE_DIR} -name *.cpp) | tail -1 | awk '{print $1}'

echo -e "\nTotal number of *.h files:"
find ${SOURCE_DIR} -name *.h | wc -l

echo -e "\nTotal number of lines in *.h files:"
wc -l $(find ${SOURCE_DIR} -name *.h) | tail -1 | awk '{print $1}'

