#pragma once

#include <innodb/univ/univ.h>
#include <innodb/univ/rec_t.h>

struct buf_block_t;

/** Updates the lock table when a page is merged to the left. */
void lock_update_merge_left(
    const buf_block_t *left_block,   /*!< in: left page to
                                     which merged */
    const rec_t *orig_pred,          /*!< in: original predecessor
                                     of supremum on the left page
                                     before merge */
    const buf_block_t *right_block); /*!< in: merged index page
                                     which will be discarded */
