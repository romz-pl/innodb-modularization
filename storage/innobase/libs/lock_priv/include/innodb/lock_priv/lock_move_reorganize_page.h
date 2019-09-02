#pragma once

#include <innodb/univ/univ.h>

struct buf_block_t;

/** Updates the lock table when we have reorganized a page. NOTE: we copy
 also the locks set on the infimum of the page; the infimum may carry
 locks if an update of a record is occurring on the page, and its locks
 were temporarily stored on the infimum. */
void lock_move_reorganize_page(
    const buf_block_t *block,   /*!< in: old index page, now
                                reorganized */
    const buf_block_t *oblock); /*!< in: copy of the old, not
                                reorganized page */
