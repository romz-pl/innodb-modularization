#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_table_t.h>

/** Gets the given system column number of a table.
 @return column number */
UNIV_INLINE
ulint dict_table_get_sys_col_no(const dict_table_t *table, /*!< in: table */
                                ulint sys) /*!< in: DATA_ROW_ID, ... */
{
  ut_ad(table);
  ut_ad(sys < table->get_n_sys_cols());
  ut_ad(table->magic_n == DICT_TABLE_MAGIC_N);

  return (table->n_cols - table->get_n_sys_cols() + sys);
}
