#include <innodb/hash/hash_table_clear.h>

#include <innodb/hash/hash_table_t.h>
#include <innodb/hash/hash_cell_t.h>

/** Clears a hash table so that all the cells become empty. */
void hash_table_clear(hash_table_t *table) /*!< in/out: hash table */
{
  ut_ad(table);
  ut_ad(table->magic_n == HASH_TABLE_MAGIC_N);
  memset(table->cells, 0x0, table->n_cells * sizeof(*table->cells));
}
