#pragma once

#include <innodb/univ/univ.h>

/** Calculates the checksum for a log block using the MySQL 5.7 algorithm.
@param[in]	log_block	log block
@return checksum */
uint32_t log_block_calc_checksum_crc32(const byte *log_block);
