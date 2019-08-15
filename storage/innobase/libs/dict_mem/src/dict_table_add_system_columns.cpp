#include <innodb/dict_mem/dict_table_add_system_columns.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_mem_table_add_col.h>
#include <innodb/dict_mem/flags.h>
#include <innodb/data_types/flags.h>
#include <innodb/data_types/flags.h>

/** Adds system columns to a table object. */
void dict_table_add_system_columns(dict_table_t *table, /*!< in/out: table */
                                   mem_heap_t *heap) /*!< in: temporary heap */
{
  ut_ad(table);
  ut_ad(table->n_def == (table->n_cols - table->get_n_sys_cols()));
  ut_ad(table->magic_n == DICT_TABLE_MAGIC_N);
  ut_ad(!table->cached);

  /* NOTE: the system columns MUST be added in the following order
  (so that they can be indexed by the numerical value of DATA_ROW_ID,
  etc.) and as the last columns of the table memory object.
  The clustered index will not always physically contain all system
  columns.
  Intrinsic table don't need DB_ROLL_PTR as UNDO logging is turned off
  for these tables. */

  dict_mem_table_add_col(table, heap, "DB_ROW_ID", DATA_SYS,
                         DATA_ROW_ID | DATA_NOT_NULL, DATA_ROW_ID_LEN);

  dict_mem_table_add_col(table, heap, "DB_TRX_ID", DATA_SYS,
                         DATA_TRX_ID | DATA_NOT_NULL, DATA_TRX_ID_LEN);

  if (!table->is_intrinsic()) {
    dict_mem_table_add_col(table, heap, "DB_ROLL_PTR", DATA_SYS,
                           DATA_ROLL_PTR | DATA_NOT_NULL, DATA_ROLL_PTR_LEN);

    /* This check reminds that if a new system column is added to
    the program, it should be dealt with here */
  }
}
