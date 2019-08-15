#include <innodb/dict_mem/dict_mem_table_add_s_col.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/memory/mem_heap_zalloc.h>

/** Adds a stored column definition to a table.
@param[in]	table		table
@param[in]	num_base	number of base columns. */
void dict_mem_table_add_s_col(dict_table_t *table, ulint num_base) {
  ulint i = table->n_def - 1;
  dict_col_t *col = table->get_col(i);
  dict_s_col_t s_col;

  ut_ad(col != NULL);

  if (table->s_cols == NULL) {
    table->s_cols = UT_NEW_NOKEY(dict_s_col_list());
  }

  s_col.m_col = col;
  s_col.s_pos = i + table->n_v_def;

  if (num_base != 0) {
    s_col.base_col = static_cast<dict_col_t **>(
        mem_heap_zalloc(table->heap, num_base * sizeof(dict_col_t *)));
  } else {
    s_col.base_col = NULL;
  }

  s_col.num_base = num_base;
  table->s_cols->push_back(s_col);
}
