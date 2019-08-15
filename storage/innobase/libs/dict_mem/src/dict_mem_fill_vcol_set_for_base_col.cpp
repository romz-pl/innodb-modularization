#include <innodb/dict_mem/dict_mem_fill_vcol_set_for_base_col.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_vcol_set.h>
#include <innodb/dict_mem/dict_table_get_nth_v_col.h>
#include <innodb/dict_mem/dict_v_col_t.h>

/** Fill the virtual column set with virtual columns which have base columns
as the given col_name
@param[in]	col_name	column name
@param[in]	table		table object
@param[out]	v_cols		set of virtual columns. */
void dict_mem_fill_vcol_set_for_base_col(const char *col_name,
                                                const dict_table_t *table,
                                                dict_vcol_set **v_cols) {
  for (ulint i = 0; i < table->n_v_cols; i++) {
    dict_v_col_t *v_col = dict_table_get_nth_v_col(table, i);

    if (!v_col->m_col.ord_part) {
      continue;
    }

    for (ulint j = 0; j < v_col->num_base; j++) {
      if (strcmp(col_name, table->get_col_name(v_col->base_col[j]->ind)) == 0) {
        if (*v_cols == NULL) {
          *v_cols = UT_NEW_NOKEY(dict_vcol_set());
        }

        (*v_cols)->insert(v_col);
      }
    }
  }
}
