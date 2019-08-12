#pragma once

#include <innodb/univ/univ.h>

/** In versions < 4.0.14 and < 4.1.1 there was a bug that the checksum only
 looked at the first few bytes of the page. This calculates that old
 checksum.
 NOTE: we must first store the new formula checksum to
 FIL_PAGE_SPACE_OR_CHKSUM before calculating and storing this old checksum
 because this takes that field as an input!
 @return checksum */
ulint buf_calc_page_old_checksum(const byte *page); /*!< in: buffer page */
