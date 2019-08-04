#include <innodb/hash/hash_get_nth_lock.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/hash/hash_table_t.h>

/** Gets the nth rw_lock in a hash table.
 @return rw_lock */
rw_lock_t *hash_get_nth_lock(hash_table_t *table, /*!< in: hash table */
                             ulint i) /*!< in: index of the rw_lock */
{
  ut_ad(table);
  ut_ad(table->magic_n == HASH_TABLE_MAGIC_N);
  ut_ad(table->type == HASH_TABLE_SYNC_RW_LOCK);
  ut_ad(i < table->n_sync_obj);

  return (table->sync_obj.rw_locks + i);
}

#endif
