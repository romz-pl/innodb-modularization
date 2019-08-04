#include <innodb/hash/hash_get_heap.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/hash/hash_table_t.h>
#include <innodb/hash/hash_get_sync_obj_index.h>
#include <innodb/hash/hash_get_nth_heap.h>

/** Gets the heap for a fold value in a hash table.
 @return mem heap */
mem_heap_t *hash_get_heap(hash_table_t *table, /*!< in: hash table */
                          ulint fold)          /*!< in: fold */
{
  ulint i;

  ut_ad(table);
  ut_ad(table->magic_n == HASH_TABLE_MAGIC_N);

  if (table->heap) {
    return (table->heap);
  }

  i = hash_get_sync_obj_index(table, fold);

  return (hash_get_nth_heap(table, i));
}

#endif
