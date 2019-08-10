#pragma once

#include <innodb/univ/univ.h>

/** Sets value of a log block checksum field.
@param[in,out]	log_block	log block
@param[in]	checksum	checksum */
void log_block_set_checksum(byte *log_block, uint32_t checksum);
