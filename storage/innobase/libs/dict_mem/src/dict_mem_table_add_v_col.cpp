#include <innodb/dict_mem/dict_mem_table_add_v_col.h>

#include <innodb/dict_mem/dict_add_col_name.h>
#include <innodb/dict_mem/dict_mem_fill_column_struct.h>
#include <innodb/dict_mem/dict_table_get_nth_v_col.h>
#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_v_col_t.h>
#include <innodb/memory/mem_heap_zalloc.h>

/** Adds a virtual column definition to a table.
@param[in,out]	table		table
@param[in,out]	heap		temporary memory heap, or NULL. It is
                                used to store name when we have not finished
                                adding all columns. When all columns are
                                added, the whole name will copy to memory from
                                table->heap
@param[in]	name		column name
@param[in]	mtype		main datatype
@param[in]	prtype		precise type
@param[in]	len		length
@param[in]	pos		position in a table
@param[in]	num_base	number of base columns
@return the virtual column definition */
dict_v_col_t *dict_mem_table_add_v_col(dict_table_t *table, mem_heap_t *heap,
                                       const char *name, ulint mtype,
                                       ulint prtype, ulint len, ulint pos,
                                       ulint num_base) {
  dict_v_col_t *v_col;
  ulint i;

  ut_ad(table);
  ut_ad(table->magic_n == DICT_TABLE_MAGIC_N);
  ut_ad(!heap == !name);

  ut_ad(prtype & DATA_VIRTUAL);

  i = table->n_v_def++;

  table->n_t_def++;

  if (name != NULL) {
    if (table->n_v_def == table->n_v_cols) {
      heap = table->heap;
    }

    if (i && !table->v_col_names) {
      /* All preceding column names are empty. */
      char *s = static_cast<char *>(mem_heap_zalloc(heap, table->n_v_def));

      table->v_col_names = s;
    }

    table->v_col_names = dict_add_col_name(table->v_col_names, i, name, heap);
  }

  v_col = dict_table_get_nth_v_col(table, i);

  dict_mem_fill_column_struct(&v_col->m_col, pos, mtype, prtype, len);
  v_col->v_pos = i;

  if (num_base != 0) {
    v_col->base_col = static_cast<dict_col_t **>(
        mem_heap_zalloc(table->heap, num_base * sizeof(*v_col->base_col)));
  } else {
    v_col->base_col = NULL;
  }

  v_col->num_base = num_base;

  /* Initialize the index list for virtual columns */
  v_col->v_indexes = UT_NEW_NOKEY(dict_v_idx_list());

  return (v_col);
}
