#include <innodb/dict_mem/dict_mem_table_add_col.h>

#include <innodb/memory/mem_heap_zalloc.h>
#include <innodb/dict_mem/dict_add_col_name.h>
#include <innodb/dict_mem/dict_mem_fill_column_struct.h>
#include <innodb/dict_mem/dict_table_t.h>

/** Adds a column definition to a table. */
void dict_mem_table_add_col(
    dict_table_t *table, /*!< in: table */
    mem_heap_t *heap,    /*!< in: temporary memory heap, or NULL */
    const char *name,    /*!< in: column name, or NULL */
    ulint mtype,         /*!< in: main datatype */
    ulint prtype,        /*!< in: precise type */
    ulint len)           /*!< in: precision */
{
  dict_col_t *col;
  ulint i;

  ut_ad(table);
  ut_ad(table->magic_n == DICT_TABLE_MAGIC_N);
  ut_ad(!heap == !name);

  ut_ad(!(prtype & DATA_VIRTUAL));

  i = table->n_def++;

  table->n_t_def++;

  if (name) {
    if (table->n_def == table->n_cols) {
      heap = table->heap;
    }
    if (i && !table->col_names) {
      /* All preceding column names are empty. */
      char *s = static_cast<char *>(mem_heap_zalloc(heap, table->n_def));

      table->col_names = s;
    }

    table->col_names = dict_add_col_name(table->col_names, i, name, heap);
  }

  col = table->get_col(i);

  dict_mem_fill_column_struct(col, i, mtype, prtype, len);
}

