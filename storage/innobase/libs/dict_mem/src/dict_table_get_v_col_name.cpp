#include <innodb/dict_mem/dict_table_get_v_col_name.h>

#include <innodb/dict_mem/dict_table_t.h>

/** Returns a virtual column's name.
@param[in]	table	target table
@param[in]	col_nr	virtual column number (nth virtual column)
@return column name or NULL if column number out of range. */
const char *dict_table_get_v_col_name(const dict_table_t *table, ulint col_nr) {
  const char *s;

  ut_ad(table);
  ut_ad(col_nr < table->n_v_def);
  ut_ad(table->magic_n == DICT_TABLE_MAGIC_N);

  if (col_nr >= table->n_v_def) {
    return (NULL);
  }

  s = table->v_col_names;

  if (s != NULL) {
    for (ulint i = 0; i < col_nr; i++) {
      s += strlen(s) + 1;
    }
  }

  return (s);
}
