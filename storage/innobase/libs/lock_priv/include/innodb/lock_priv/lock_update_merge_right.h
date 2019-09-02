#pragma once

#include <innodb/univ/univ.h>
#include <innodb/univ/rec_t.h>

struct buf_block_t;

/** Updates the lock table when a page is merged to the right. */
void lock_update_merge_right(
    const buf_block_t *right_block, /*!< in: right page to
                                    which merged */
    const rec_t *orig_succ,         /*!< in: original
                                    successor of infimum
                                    on the right page
                                    before merge */
    const buf_block_t *left_block); /*!< in: merged index
                                    page which will be
                                    discarded */
