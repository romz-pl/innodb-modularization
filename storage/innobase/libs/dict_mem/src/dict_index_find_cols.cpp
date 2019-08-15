#include <innodb/dict_mem/dict_index_find_cols.h>

#include <innodb/dict_mem/dict_add_v_col_t.h>
#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_table_get_nth_v_col.h>
#include <innodb/dict_mem/dict_table_get_v_col_name.h>
#include <innodb/dict_mem/dict_v_col_t.h>

#include <algorithm>

/** Tries to find column names for the index and sets the col field of the
index.
@param[in]	table	table
@param[in,out]	index	index
@param[in]	add_v	new virtual columns added along with an add index call
@return true if the column names were found */
ibool dict_index_find_cols(const dict_table_t *table,
                                  dict_index_t *index,
                                  const dict_add_v_col_t *add_v) {
  std::vector<ulint, ut_allocator<ulint>> col_added;
  std::vector<ulint, ut_allocator<ulint>> v_col_added;

  ut_ad(table != NULL && index != NULL);
  ut_ad(table->magic_n == DICT_TABLE_MAGIC_N);

  for (ulint i = 0; i < index->n_fields; i++) {
    ulint j;
    dict_field_t *field = index->get_field(i);

    for (j = 0; j < table->n_cols; j++) {
      if (!strcmp(table->get_col_name(j), field->name)) {
        /* Check if same column is being assigned again
        which suggest that column has duplicate name. */
        bool exists =
            std::find(col_added.begin(), col_added.end(), j) != col_added.end();

        if (exists) {
          /* Duplicate column found. */
          goto dup_err;
        }

        field->col = table->get_col(j);

        col_added.push_back(j);

        goto found;
      }
    }

    /* Let's check if it is a virtual column */
    for (j = 0; j < table->n_v_cols; j++) {
      if (!strcmp(dict_table_get_v_col_name(table, j), field->name)) {
        /* Check if same column is being assigned again
        which suggest that column has duplicate name. */
        bool exists = std::find(v_col_added.begin(), v_col_added.end(), j) !=
                      v_col_added.end();

        if (exists) {
          /* Duplicate column found. */
          break;
        }

        field->col =
            reinterpret_cast<dict_col_t *>(dict_table_get_nth_v_col(table, j));

        v_col_added.push_back(j);

        goto found;
      }
    }

    if (add_v) {
      for (j = 0; j < add_v->n_v_col; j++) {
        if (!strcmp(add_v->v_col_name[j], field->name)) {
          field->col = const_cast<dict_col_t *>(&add_v->v_col[j].m_col);
          goto found;
        }
      }
    }

  dup_err:
#ifdef UNIV_DEBUG
    /* It is an error not to find a matching column. */
    ib::error(ER_IB_MSG_182)
        << "No matching column for " << field->name << " in index "
        << index->name << " of table " << table->name;
#endif /* UNIV_DEBUG */
    return (FALSE);

  found:;
  }

  return (TRUE);
}
