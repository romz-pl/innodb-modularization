#pragma once

#include <innodb/univ/univ.h>

/** Stores a 4-byte checksum to the trailer checksum field of a log block.
This is used before writing the log block to disk. The checksum in a log
block is used in recovery to check the consistency of the log block.
@param[in]	log_block	 log block (completely filled in!) */
void log_block_store_checksum(byte *log_block);
