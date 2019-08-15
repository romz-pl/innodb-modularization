#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_table_get_n_v_cols.h>

/** Gets the number of user-defined virtual and non-virtual columns in a table
in the dictionary cache.
@param[in]	table	table
@return number of user-defined (e.g., not ROW_ID) columns of a table */
UNIV_INLINE
ulint dict_table_get_n_tot_u_cols(const dict_table_t *table) {
  ut_ad(table);
  ut_ad(table->magic_n == DICT_TABLE_MAGIC_N);

  return (table->get_n_user_cols() + dict_table_get_n_v_cols(table));
}
