#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_heap_t.h>

struct dict_index_t;
struct dtuple_t;

/** Builds a partial row from an update undo log record, for purge.
 It contains the columns which occur as ordering in any index of the table.
 Any missing columns are indicated by col->mtype == DATA_MISSING.
 @return pointer to remaining part of undo record */
byte *trx_undo_rec_get_partial_row(
    const byte *ptr,     /*!< in: remaining part in update undo log
                         record of a suitable type, at the start of
                         the stored index columns;
                         NOTE that this copy of the undo log record must
                         be preserved as long as the partial row is
                         used, as we do NOT copy the data in the
                         record! */
    dict_index_t *index, /*!< in: clustered index */
    dtuple_t **row,      /*!< out, own: partial row */
    ibool ignore_prefix, /*!< in: flag to indicate if we
                  expect blob prefixes in undo. Used
                  only in the assertion. */
    mem_heap_t *heap)    /*!< in: memory heap from which the memory
                         needed is allocated */
    MY_ATTRIBUTE((warn_unused_result));
