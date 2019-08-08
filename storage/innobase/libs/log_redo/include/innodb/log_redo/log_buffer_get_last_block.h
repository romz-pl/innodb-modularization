#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/lsn_t.h>

struct log_t;

/** Get last redo block from redo buffer and end LSN. Note that it takes
x-lock on the log buffer for a short period. Out values are always set,
even when provided last_block is nullptr.
@param[in,out]	log		redo log
@param[out]	last_lsn	end lsn of last mtr
@param[out]	last_block	last redo block
@param[in,out]	block_len	length in bytes */
void log_buffer_get_last_block(log_t &log, lsn_t &last_lsn, byte *last_block,
                               uint32_t &block_len);
