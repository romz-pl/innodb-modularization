#pragma once

#include <innodb/univ/univ.h>

/** Sets the log block data length.
@param[in,out]	log_block	log block
@param[in]	len		data length (@see log_block_get_data_len) */
void log_block_set_data_len(byte *log_block, ulint len);
