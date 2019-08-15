#include <innodb/dict_mem/dict_table_set_big_rows.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/flags.h>
#include <innodb/data_types/flags.h>

/** Mark if table has big rows.
@param[in,out]	table	table handler */
void dict_table_set_big_rows(dict_table_t *table) {
  ulint row_len = 0;
  for (ulint i = 0; i < table->n_def; i++) {
    ulint col_len = table->get_col(i)->get_max_size();

    row_len += col_len;

    /* If we have a single unbounded field, or several gigantic
    fields, mark the maximum row size as BIG_ROW_SIZE. */
    if (row_len >= BIG_ROW_SIZE || col_len >= BIG_ROW_SIZE) {
      row_len = BIG_ROW_SIZE;

      break;
    }
  }

  table->big_rows = (row_len >= BIG_ROW_SIZE) ? TRUE : FALSE;
}
