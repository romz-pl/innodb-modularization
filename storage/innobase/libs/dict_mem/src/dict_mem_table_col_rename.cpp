#include <innodb/dict_mem/dict_mem_table_col_rename.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_mem_table_col_rename_low.h>

/** Renames a column of a table in the data dictionary cache. */
void dict_mem_table_col_rename(dict_table_t *table, /*!< in/out: table */
                               ulint nth_col,       /*!< in: column index */
                               const char *from,    /*!< in: old column name */
                               const char *to,      /*!< in: new column name */
                               bool is_virtual)
/*!< in: if this is a virtual column */
{
  const char *s = is_virtual ? table->v_col_names : table->col_names;

  ut_ad((!is_virtual && nth_col < table->n_def) ||
        (is_virtual && nth_col < table->n_v_def));

  for (ulint i = 0; i < nth_col; i++) {
    size_t len = strlen(s);
    ut_ad(len > 0);
    s += len + 1;
  }

  /* This could fail if the data dictionaries are out of sync.
  Proceed with the renaming anyway. */
  ut_ad(!strcmp(from, s));

  dict_mem_table_col_rename_low(table, static_cast<unsigned>(nth_col), to, s,
                                is_virtual);
}
