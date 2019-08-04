#include <innodb/hash/hash_get_nth_mutex.h>


#ifndef UNIV_HOTBACKUP

#include <innodb/hash/hash_table_t.h>

/** Gets the nth mutex in a hash table.
 @return mutex */
ib_mutex_t *hash_get_nth_mutex(hash_table_t *table, /*!< in: hash table */
                               ulint i) /*!< in: index of the mutex */
{
  ut_ad(table);
  ut_ad(table->magic_n == HASH_TABLE_MAGIC_N);
  ut_ad(table->type == HASH_TABLE_SYNC_MUTEX);
  ut_ad(i < table->n_sync_obj);

  return (table->sync_obj.mutexes + i);
}

#endif
