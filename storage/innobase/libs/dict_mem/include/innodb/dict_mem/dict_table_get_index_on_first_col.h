#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_table_get_nth_v_col.h>
#include <innodb/dict_mem/dict_v_col_t.h>

/** Get index by first field of the index
 @return index which is having first field matches
 with the field present in field_index position of table */
UNIV_INLINE
dict_index_t *dict_table_get_index_on_first_col(
    dict_table_t *table, /*!< in: table */
    ulint col_index)     /*!< in: position of column
                         in table */
{
  ut_ad(col_index < table->n_cols);

  ulint non_v_col_index = col_index;
  dict_col_t *column = NULL;

  /** Decrement the col_index if the virtual column encountered
  before the col_index of the table. */
  for (ulint i = 0; i < table->n_v_cols; i++) {
    column = reinterpret_cast<dict_col_t *>(dict_table_get_nth_v_col(table, i));

    if (column->ind >= col_index) {
      break;
    }

    non_v_col_index--;
  }

  column = table->get_col(non_v_col_index);

  for (dict_index_t *index = table->first_index(); index != NULL;
       index = index->next()) {
    if (index->fields[0].col == column) {
      return (index);
    }
  }
  ut_error;
}
