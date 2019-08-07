#pragma once

#include <innodb/univ/univ.h>

/** Gets an offset to the beginning of the first group of log records
in a given log block.
@param[in]	log_block	log block
@return first mtr log record group byte offset from the block start,
0 if none. */
uint32_t log_block_get_first_rec_group(const byte *log_block);
