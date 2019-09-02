#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

struct buf_block_t;

/** Moves the explicit locks on user records to another page if a record
 list start is moved to another page. */
void lock_move_rec_list_start(
    const buf_block_t *new_block, /*!< in: index page to move to */
    const buf_block_t *block,     /*!< in: index page */
    const rec_t *rec,             /*!< in: record on page:
                                  this is the first
                                  record NOT copied */
    const rec_t *old_end);        /*!< in: old
                                  previous-to-last
                                  record on new_page
                                  before the records
                                  were copied */
