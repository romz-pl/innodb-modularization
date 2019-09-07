#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_types/trx_undo_rec_t.h>
#include <innodb/memory/mem_heap_t.h>

struct trx_t;

/** Get next undo log record from redo and noredo rollback segments.
 @return undo log record copied to heap, NULL if none left, or if the
 undo number of the top record would be less than the limit */
trx_undo_rec_t *trx_roll_pop_top_rec_of_trx(
    trx_t *trx,           /*!< in: transaction */
    undo_no_t limit,      /*!< in: least undo number we need */
    roll_ptr_t *roll_ptr, /*!< out: roll pointer to undo record */
    mem_heap_t *heap);    /*!< in: memory heap where copied */
