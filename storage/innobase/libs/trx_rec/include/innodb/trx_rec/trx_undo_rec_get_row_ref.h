#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_heap_t.h>

struct dict_index_t;
struct dtuple_t;

/** Builds a row reference from an undo log record.
 @return pointer to remaining part of undo record */
byte *trx_undo_rec_get_row_ref(
    byte *ptr,           /*!< in: remaining part of a copy of an undo log
                         record, at the start of the row reference;
                         NOTE that this copy of the undo log record must
                         be preserved as long as the row reference is
                         used, as we do NOT copy the data in the
                         record! */
    dict_index_t *index, /*!< in: clustered index */
    dtuple_t **ref,      /*!< out, own: row reference */
    mem_heap_t *heap);   /*!< in: memory heap from which the memory
                         needed is allocated */
