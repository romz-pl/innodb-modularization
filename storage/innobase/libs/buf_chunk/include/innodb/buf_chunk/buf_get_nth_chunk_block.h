#pragma once

#include <innodb/univ/univ.h>

struct buf_block_t;
struct buf_pool_t;
buf_block_t *buf_get_nth_chunk_block(const buf_pool_t *buf_pool, ulint n, ulint *chunk_size);
