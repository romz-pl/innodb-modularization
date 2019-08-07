#pragma once

#include <innodb/univ/univ.h>

/** Gets a log block flush bit. The flush bit is set, if and only if,
the block was the first block written in a call to fil_io().

During recovery, when encountered the flush bit, recovery code can be
pretty sure, that all previous blocks belong to a completed fil_io(),
because the block with flush bit belongs to the next call to fil_io(),
which could only be started after the previous one has been finished.

@param[in]	log_block	log block
@return true if this block was the first to be written in fil_io(). */
bool log_block_get_flush_bit(const byte *log_block);
