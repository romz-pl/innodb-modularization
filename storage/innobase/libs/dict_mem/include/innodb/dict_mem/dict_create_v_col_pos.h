#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/flags.h>
#include <innodb/assert/assert.h>

/** Compose a column number for a virtual column, stored in the "POS" field
of Sys_columns. The column number includes both its virtual column sequence
(the "nth" virtual column) and its actual column position in original table
@param[in]	v_pos		virtual column sequence
@param[in]	col_pos		column position in original table definition
@return composed column position number */
UNIV_INLINE
ulint dict_create_v_col_pos(ulint v_pos, ulint col_pos) {
  ut_ad(v_pos <= REC_MAX_N_FIELDS);
  ut_ad(col_pos <= REC_MAX_N_FIELDS);

  return (((v_pos + 1) << 16) + col_pos);
}
