#pragma once

#include <innodb/univ/univ.h>

/** Calculates the checksum for a log block.
@param[in]	log_block	log block
@return checksum */
uint32_t log_block_calc_checksum(const byte *log_block);
