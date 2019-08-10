#pragma once

#include <innodb/univ/univ.h>

/** Gets value of a log block checksum field.
@param[in]	log_block	log block
@return checksum */
uint32_t log_block_get_checksum(const byte *log_block);
