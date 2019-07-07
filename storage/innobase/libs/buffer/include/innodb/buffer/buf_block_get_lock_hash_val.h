#pragma once

#include <innodb/univ/univ.h>

struct buf_block_t;
ulint buf_block_get_lock_hash_val(const buf_block_t *block);
