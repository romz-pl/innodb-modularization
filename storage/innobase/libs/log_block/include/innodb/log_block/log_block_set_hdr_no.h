#pragma once

#include <innodb/univ/univ.h>

/** Sets the log block number stored in the header.
NOTE that this must be set before the flush bit!

@param[in,out]	log_block	log block
@param[in]	n		log block number: must be in (0, 1G] */
void log_block_set_hdr_no(byte *log_block, uint32_t n);
