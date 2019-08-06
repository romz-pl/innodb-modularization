#pragma once

#include <innodb/univ/univ.h>

#include <innodb/mtr/mtr_t.h>

/** Check if a mini-transaction is dirtying a clean page.
@param b	block being x-fixed
@return true if the mtr is dirtying a clean page. */
#define mtr_block_dirtied(b) mtr_t::is_block_dirtied((b))
