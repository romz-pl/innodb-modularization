#pragma once

#include <innodb/univ/univ.h>

/** Calculates the checksum for a log block using the "no-op" algorithm.
@param[in]     log_block   log block
@return        checksum */
uint32_t log_block_calc_checksum_none(const byte *log_block);
