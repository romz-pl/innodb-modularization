#include <innodb/hash/hash_table_free.h>

#include <innodb/hash/hash_table_t.h>
#include <innodb/allocator/ut_free.h>

/** Frees a hash table. */
void hash_table_free(hash_table_t *table) /*!< in, own: hash table */
{
  ut_ad(table->magic_n == HASH_TABLE_MAGIC_N);

  ut_free(table->cells);
  ut_free(table);
}
