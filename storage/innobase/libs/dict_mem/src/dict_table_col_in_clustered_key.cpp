#include <innodb/dict_mem/dict_table_col_in_clustered_key.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/dict_mem/dict_field_t.h>
#include <innodb/dict_mem/dict_index_get_n_unique.h>

/** Checks if a column is in the ordering columns of the clustered index of a
 table. Column prefixes are treated like whole columns.
 @return true if the column, or its prefix, is in the clustered key */
ibool dict_table_col_in_clustered_key(
    const dict_table_t *table, /*!< in: table */
    ulint n)                   /*!< in: column number */
{
  const dict_index_t *index;
  const dict_field_t *field;
  const dict_col_t *col;
  ulint pos;
  ulint n_fields;

  ut_ad(table);

  col = table->get_col(n);

  index = table->first_index();

  n_fields = dict_index_get_n_unique(index);

  for (pos = 0; pos < n_fields; pos++) {
    field = index->get_field(pos);

    if (col == field->col) {
      return (TRUE);
    }
  }

  return (FALSE);
}
