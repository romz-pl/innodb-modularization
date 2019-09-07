#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_types/trx_undo_rec_t.h>
#include <innodb/memory/mem_heap_t.h>

/** Copies the undo record to the heap.
@param[in]	undo_rec	undo log record
@param[in]	heap		heap where copied
@return own: copy of undo log record */
trx_undo_rec_t *trx_undo_rec_copy(const trx_undo_rec_t *undo_rec,
                                  mem_heap_t *heap);
