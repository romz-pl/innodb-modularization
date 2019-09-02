#pragma once

#include <innodb/univ/univ.h>

struct buf_block_t;

/** Updates the lock table when a page is copied to another and the original
 page is removed from the chain of leaf pages, except if page is the root! */
void lock_update_copy_and_discard(
    const buf_block_t *new_block, /*!< in: index page to
                                  which copied */
    const buf_block_t *block);    /*!< in: index page;
                                  NOT the root! */
