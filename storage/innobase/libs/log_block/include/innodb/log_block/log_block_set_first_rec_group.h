#pragma once

#include <innodb/univ/univ.h>

/** Sets an offset to the beginning of the first group of log records
in a given log block.
@param[in,out]	log_block	log block
@param[in]	offset		offset, 0 if none */
void log_block_set_first_rec_group(byte *log_block, uint32_t offset);
