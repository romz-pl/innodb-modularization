#include <innodb/hash/hash_get_n_cells.h>

#include <innodb/hash/hash_table_t.h>

/** Returns the number of cells in a hash table.
 @return number of cells */
ulint hash_get_n_cells(hash_table_t *table) /*!< in: table */
{
  ut_ad(table);
  ut_ad(table->magic_n == HASH_TABLE_MAGIC_N);
  return (table->n_cells);
}
