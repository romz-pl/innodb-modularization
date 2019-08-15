#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_table_get_nth_v_col.h>
#include <innodb/dict_mem/dict_table_has_atomic_blobs.h>
#include <innodb/dict_mem/flags.h>
#include <innodb/dict_mem/dict_col_t.h>
#include <innodb/dict_mem/dict_v_col_t.h>

/** Determine maximum bytes of a virtual column need to be stored
in the undo log.
@param[in]	table		dict_table_t for the table
@param[in]	col_no		virtual column number
@return maximum bytes of virtual column to be stored in the undo log */
UNIV_INLINE
ulint dict_max_v_field_len_store_undo(dict_table_t *table, ulint col_no) {
  const dict_col_t *col = &dict_table_get_nth_v_col(table, col_no)->m_col;
  ulint max_log_len;

  /* This calculation conforms to the non-virtual column
  maximum log length calculation:
  1) if No atomic BLOB, upto REC_ANTELOPE_MAX_INDEX_COL_LEN
  2) if atomic BLOB, upto col->max_prefix or
  REC_VERSION_56_MAX_INDEX_COL_LEN, whichever is less */
  if (dict_table_has_atomic_blobs(table)) {
    if (DATA_BIG_COL(col) && col->max_prefix > 0) {
      max_log_len = col->max_prefix;
    } else {
      max_log_len = DICT_MAX_FIELD_LEN_BY_FORMAT(table);
    }
  } else {
    max_log_len = REC_ANTELOPE_MAX_INDEX_COL_LEN;
  }

  return (max_log_len);
}
