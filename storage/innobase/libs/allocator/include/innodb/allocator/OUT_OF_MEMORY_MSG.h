#pragma once

#include <innodb/univ/univ.h>

#define OUT_OF_MEMORY_MSG                                             \
  "Check if you should increase the swap file or ulimits of your"     \
  " operating system. Note that on most 32-bit computers the process" \
  " memory space is limited to 2 GB or 4 GB."
