#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_table_get_n_v_cols.h>
#include <innodb/dict_mem/dict_table_get_nth_v_col.h>
#include <innodb/dict_mem/dict_v_col_t.h>

/** Check whether the table have virtual index.
@param[in]	table	InnoDB table
@return true if the table have virtual index, false otherwise. */
UNIV_INLINE
bool dict_table_have_virtual_index(dict_table_t *table) {
  for (ulint col_no = 0; col_no < dict_table_get_n_v_cols(table); col_no++) {
    const dict_v_col_t *col = dict_table_get_nth_v_col(table, col_no);

    if (col->m_col.ord_part) {
      return (true);
    }
  }

  return (false);
}
