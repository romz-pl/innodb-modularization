#pragma once

#include <innodb/univ/univ.h>


/** Calculates a page checksum which is stored to the page when it is written
 to a file. Note that we must be careful to calculate the same value on
 32-bit and 64-bit architectures.
 @return checksum */
ulint buf_calc_page_new_checksum(const byte *page); /*!< in: buffer page */
