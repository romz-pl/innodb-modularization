#pragma once

#include <innodb/univ/univ.h>

/** Sets the log block flush bit.
@param[in,out]	log_block	log block (must have hdr_no != 0)
@param[in]	value		value to set */
void log_block_set_flush_bit(byte *log_block, bool value);
