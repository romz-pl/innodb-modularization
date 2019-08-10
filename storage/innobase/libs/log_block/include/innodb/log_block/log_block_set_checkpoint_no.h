#pragma once

#include <innodb/univ/univ.h>

/** Sets a log block checkpoint number field (4 lowest bytes).
@param[in,out]	log_block	log block
@param[in]	no		checkpoint no */
void log_block_set_checkpoint_no(byte *log_block, uint64_t no);
