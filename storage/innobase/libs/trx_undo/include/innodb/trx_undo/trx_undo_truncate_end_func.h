#pragma once

#include <innodb/univ/univ.h>

struct trx_undo_t;

/** Truncates an undo log from the end. This function is used during a rollback
 to free space from an undo log. */
void trx_undo_truncate_end_func(
#ifdef UNIV_DEBUG
    const trx_t *trx, /*!< in: transaction whose undo log it is */
#endif                /* UNIV_DEBUG */
    trx_undo_t *undo, /*!< in/out: undo log */
    undo_no_t limit); /*!< in: all undo records with undo number
                      >= this value should be truncated */
