#pragma once

#include <innodb/univ/univ.h>

struct buf_block_t;

/** Updates the lock table when a page is split to the right. */
void lock_update_split_right(
    const buf_block_t *right_block, /*!< in: right page */
    const buf_block_t *left_block); /*!< in: left page */
