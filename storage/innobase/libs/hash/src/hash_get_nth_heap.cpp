#include <innodb/hash/hash_get_nth_heap.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/hash/hash_table_t.h>

/** Gets the nth heap in a hash table.
 @return mem heap */
mem_heap_t *hash_get_nth_heap(hash_table_t *table, /*!< in: hash table */
                              ulint i)             /*!< in: index of the heap */
{
  ut_ad(table);
  ut_ad(table->magic_n == HASH_TABLE_MAGIC_N);
  ut_ad(table->type != HASH_TABLE_SYNC_NONE);
  ut_ad(i < table->n_sync_obj);

  return (table->heaps[i]);
}


#endif
