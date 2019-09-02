#pragma once

#include <innodb/univ/univ.h>

struct buf_block_t;

/** Updates the lock table when a page is discarded. */
void lock_update_discard(
    const buf_block_t *heir_block, /*!< in: index page
                                   which will inherit the locks */
    ulint heir_heap_no,            /*!< in: heap_no of the record
                                   which will inherit the locks */
    const buf_block_t *block);     /*!< in: index page
                                   which will be discarded */
