#pragma once

#include <innodb/univ/univ.h>

/** Gets a log block checkpoint number field (4 lowest bytes).
@param[in]	log_block	log block
@return checkpoint no (4 lowest bytes) */
uint32_t log_block_get_checkpoint_no(const byte *log_block);
