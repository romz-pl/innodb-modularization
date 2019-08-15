#include <innodb/dict_mem/dict_table_get_nth_v_col.h>


#ifdef UNIV_DEBUG
/** Gets the nth virtual column of a table.
@param[in]	table	table
@param[in]	pos	position of virtual column
@return pointer to virtual column object */
dict_v_col_t *dict_table_get_nth_v_col(const dict_table_t *table, ulint pos) {
  ut_ad(table);
  ut_ad(pos < table->n_v_def);
  ut_ad(table->magic_n == DICT_TABLE_MAGIC_N);

  return (static_cast<dict_v_col_t *>(table->v_cols) + pos);
}
#endif /* UNIV_DEBUG */
