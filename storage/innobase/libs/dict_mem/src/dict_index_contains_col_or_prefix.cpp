#include <innodb/dict_mem/dict_index_contains_col_or_prefix.h>

#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/dict_mem/dict_table_get_nth_v_col.h>
#include <innodb/dict_mem/dict_index_get_n_fields.h>
#include <innodb/dict_mem/dict_v_col_t.h>

/** Returns TRUE if the index contains a column or a prefix of that column.
@param[in]	index		index
@param[in]	n		column number
@param[in]	is_virtual	whether it is a virtual col
@return true if contains the column or its prefix */
ibool dict_index_contains_col_or_prefix(const dict_index_t *index, ulint n,
                                        bool is_virtual) {
  const dict_field_t *field;
  const dict_col_t *col;
  ulint pos;
  ulint n_fields;

  ut_ad(index);
  ut_ad(index->magic_n == DICT_INDEX_MAGIC_N);

  if (index->is_clustered()) {
    return (TRUE);
  }

  if (is_virtual) {
    col = &dict_table_get_nth_v_col(index->table, n)->m_col;
  } else {
    col = index->table->get_col(n);
  }

  n_fields = dict_index_get_n_fields(index);

  for (pos = 0; pos < n_fields; pos++) {
    field = index->get_field(pos);

    if (col == field->col) {
      return (TRUE);
    }
  }

  return (FALSE);
}
