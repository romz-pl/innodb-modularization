#pragma once

#include <innodb/univ/univ.h>

struct buf_pool_t;
struct buf_block_t;

buf_pool_t *buf_pool_from_block(const buf_block_t *block);
