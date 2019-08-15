#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_table_has_atomic_blobs.h>
#include <innodb/dict_mem/dict_col_t.h>
#include <innodb/dict_types/flags.h>

/** Determine bytes of column prefix to be stored in the undo log. Please
 note that if !dict_table_has_atomic_blobs(table), no prefix
 needs to be stored in the undo log.
 @return bytes of column prefix to be stored in the undo log */
UNIV_INLINE
ulint dict_max_field_len_store_undo(
    dict_table_t *table,   /*!< in: table */
    const dict_col_t *col) /*!< in: column which index prefix
                           is based on */
{
  if (!dict_table_has_atomic_blobs(table)) {
    return (0);
  }

  if (col->max_prefix != 0) {
    return (col->max_prefix);
  }

  return (REC_VERSION_56_MAX_INDEX_COL_LEN);
}
