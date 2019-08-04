#include <innodb/hash/hash_get_nth_cell.h>

#include <innodb/hash/hash_cell_t.h>
#include <innodb/hash/hash_table_t.h>
#include <innodb/assert/assert.h>

/** Gets the nth cell in a hash table.
 @return pointer to cell */
hash_cell_t *hash_get_nth_cell(hash_table_t *table, /*!< in: hash table */
                               ulint n)             /*!< in: cell index */
{
  ut_ad(table);
  ut_ad(table->magic_n == HASH_TABLE_MAGIC_N);
  ut_ad(n < table->n_cells);

  return (table->cells + n);
}
