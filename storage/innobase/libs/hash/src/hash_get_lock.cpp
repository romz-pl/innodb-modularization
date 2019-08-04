#include <innodb/hash/hash_get_lock.h>


#ifndef UNIV_HOTBACKUP

#include <innodb/hash/hash_table_t.h>
#include <innodb/hash/hash_get_sync_obj_index.h>
#include <innodb/hash/hash_get_nth_lock.h>

/** Gets the rw_lock for a fold value in a hash table.
 @return rw_lock */
rw_lock_t *hash_get_lock(hash_table_t *table, /*!< in: hash table */
                         ulint fold)          /*!< in: fold */
{
  ulint i;

  ut_ad(table);
  ut_ad(table->type == HASH_TABLE_SYNC_RW_LOCK);
  ut_ad(table->magic_n == HASH_TABLE_MAGIC_N);

  i = hash_get_sync_obj_index(table, fold);

  return (hash_get_nth_lock(table, i));
}

#endif
