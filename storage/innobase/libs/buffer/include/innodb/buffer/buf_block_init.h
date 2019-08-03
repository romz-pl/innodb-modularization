#pragma once

#include <innodb/univ/univ.h>

struct buf_pool_t;
struct buf_block_t;

void buf_block_init(
    buf_pool_t *buf_pool, /*!< in: buffer pool instance */
    buf_block_t *block,   /*!< in: pointer to control block */
    byte *frame);          /*!< in: pointer to buffer frame */
