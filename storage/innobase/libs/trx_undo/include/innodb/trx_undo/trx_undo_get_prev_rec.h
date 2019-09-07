#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/page_no_t.h>
#include <innodb/trx_types/trx_undo_rec_t.h>

struct mtr_t;

/** Gets the previous record in an undo log.
 @return undo log record, the page s-latched, NULL if none */
trx_undo_rec_t *trx_undo_get_prev_rec(
    trx_undo_rec_t *rec, /*!< in: undo record */
    page_no_t page_no,   /*!< in: undo log header page number */
    ulint offset,        /*!< in: undo log header offset on page */
    bool shared,         /*!< in: true=S-latch, false=X-latch */
    mtr_t *mtr);         /*!< in: mtr */
