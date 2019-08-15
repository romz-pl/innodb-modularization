#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_table_t.h>

/** Gets the number of virtual columns in a table in the dictionary cache.
@param[in]	table	the table to check
@return number of virtual columns of a table */
UNIV_INLINE
ulint dict_table_get_n_v_cols(const dict_table_t *table) {
  ut_ad(table);
  ut_ad(table->magic_n == DICT_TABLE_MAGIC_N);

  return (table->n_v_cols);
}
