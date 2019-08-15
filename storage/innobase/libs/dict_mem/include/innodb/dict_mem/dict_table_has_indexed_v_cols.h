#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_table_get_nth_v_col.h>
#include <innodb/dict_mem/dict_v_col_t.h>

/** Check if a table has indexed virtual columns
@param[in]	table	the table to check
@return true is the table has indexed virtual columns */
UNIV_INLINE
bool dict_table_has_indexed_v_cols(const dict_table_t *table) {
  for (ulint i = 0; i < table->n_v_cols; i++) {
    const dict_v_col_t *col = dict_table_get_nth_v_col(table, i);
    if (col->m_col.ord_part) {
      return (true);
    }
  }

  return (false);
}
