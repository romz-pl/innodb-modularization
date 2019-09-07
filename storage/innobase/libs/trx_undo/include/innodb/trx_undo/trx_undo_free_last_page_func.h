#pragma once

#include <innodb/univ/univ.h>

struct trx_undo_t;
struct mtr_t;

/** Frees the last undo log page.
 The caller must hold the rollback segment mutex. */
void trx_undo_free_last_page_func(
#ifdef UNIV_DEBUG
    const trx_t *trx, /*!< in: transaction */
#endif                /* UNIV_DEBUG */
    trx_undo_t *undo, /*!< in/out: undo log memory copy */
    mtr_t *mtr);      /*!< in/out: mini-transaction which does not
                      have a latch to any undo log page or which
                      has allocated the undo log page */
