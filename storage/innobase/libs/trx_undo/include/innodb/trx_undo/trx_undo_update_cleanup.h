#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>

struct mtr_t;
struct trx_undo_ptr_t;
struct trx_t;

/** Adds the update undo log header as the first in the history list, and
 frees the memory object, or puts it to the list of cached update undo log
 segments. */
void trx_undo_update_cleanup(
    trx_t *trx,               /*!< in: trx owning the update
                              undo log */
    trx_undo_ptr_t *undo_ptr, /*!< in: update undo log. */
    page_t *undo_page,        /*!< in: update undo log header page,
                              x-latched */
    bool update_rseg_history_len,
    /*!< in: if true: update rseg history
    len else skip updating it. */
    ulint n_added_logs, /*!< in: number of logs added */
    mtr_t *mtr);        /*!< in: mtr */
