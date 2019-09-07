#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>

struct trx_undo_t;
struct mtr_t;

/** Sets the state of the undo log segment at a transaction finish.
 @return undo log segment header page, x-latched */
page_t *trx_undo_set_state_at_finish(
    trx_undo_t *undo, /*!< in: undo log memory copy */
    mtr_t *mtr);      /*!< in: mtr */
