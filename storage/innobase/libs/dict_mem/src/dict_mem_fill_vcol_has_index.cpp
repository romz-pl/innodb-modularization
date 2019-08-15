#include <innodb/dict_mem/dict_mem_fill_vcol_has_index.h>

#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/dict_mem/dict_table_get_nth_v_col.h>
#include <innodb/dict_mem/dict_v_col_t.h>

/** Fill the virtual column set with virtual column information
present in the given virtual index.
@param[in]	index	virtual index
@param[out]	v_cols	virtual column set. */
void dict_mem_fill_vcol_has_index(const dict_index_t *index,
                                         dict_vcol_set **v_cols) {
  for (ulint i = 0; i < index->table->n_v_cols; i++) {
    dict_v_col_t *v_col = dict_table_get_nth_v_col(index->table, i);
    if (!v_col->m_col.ord_part) {
      continue;
    }

    for (auto it = v_col->v_indexes->begin(); it != v_col->v_indexes->end();
         ++it) {
      dict_v_idx_t v_idx = *it;

      if (v_idx.index == index) {
        if (*v_cols == NULL) {
          *v_cols = UT_NEW_NOKEY(dict_vcol_set());
        }

        (*v_cols)->insert(v_col);
      }
    }
  }
}
