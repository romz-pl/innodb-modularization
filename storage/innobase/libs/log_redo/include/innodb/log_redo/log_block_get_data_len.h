#pragma once

#include <innodb/univ/univ.h>

/** Gets a log block data length.
@param[in]	log_block	log block
@return log block data length measured as a byte offset from the block start */
uint32_t log_block_get_data_len(const byte *log_block);
