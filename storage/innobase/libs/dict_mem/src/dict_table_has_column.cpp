#include <innodb/dict_mem/dict_table_has_column.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/string/innobase_strcasecmp.h>

/** Check if the table has a given (non_virtual) column.
@param[in]	table		table object
@param[in]	col_name	column name
@param[in]	col_nr		column number guessed, 0 as default
@return column number if the table has the specified column,
otherwise table->n_def */
ulint dict_table_has_column(const dict_table_t *table, const char *col_name,
                            ulint col_nr) {
  ulint col_max = table->n_def;

  ut_ad(table);
  ut_ad(col_name);
  ut_ad(table->magic_n == DICT_TABLE_MAGIC_N);

  if (col_nr < col_max &&
      innobase_strcasecmp(col_name, table->get_col_name(col_nr)) == 0) {
    return (col_nr);
  }

  /** The order of column may changed, check it with other columns */
  for (ulint i = 0; i < col_max; i++) {
    if (i != col_nr &&
        innobase_strcasecmp(col_name, table->get_col_name(i)) == 0) {
      return (i);
    }
  }

  return (col_max);
}
