#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/page_no_t.h>
#include <innodb/trx_types/trx_undo_rec_t.h>

struct mtr_t;

/** Gets the next record in an undo log.
 @return undo log record, the page s-latched, NULL if none */
trx_undo_rec_t *trx_undo_get_next_rec(
    trx_undo_rec_t *rec, /*!< in: undo record */
    page_no_t page_no,   /*!< in: undo log header page number */
    ulint offset,        /*!< in: undo log header offset on page */
    mtr_t *mtr);         /*!< in: mtr */
