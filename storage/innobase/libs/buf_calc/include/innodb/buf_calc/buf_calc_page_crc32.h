#pragma once

#include <innodb/univ/univ.h>

/** Calculates the CRC32 checksum of a page. The value is stored to the page
when it is written to a file and also checked for a match when reading from
the file. When reading we allow both normal CRC32 and CRC-legacy-big-endian
variants. Note that we must be careful to calculate the same value on 32-bit
and 64-bit architectures.
@param[in]	page			buffer page (UNIV_PAGE_SIZE bytes)
@param[in]	use_legacy_big_endian	if true then use big endian
byteorder when converting byte strings to integers
@return checksum */
uint32_t buf_calc_page_crc32(const byte *page,
                             bool use_legacy_big_endian = false);
