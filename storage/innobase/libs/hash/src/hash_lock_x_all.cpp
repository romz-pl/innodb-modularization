#include <innodb/hash/hash_lock_x_all.h>


#ifndef UNIV_HOTBACKUP

#include <innodb/hash/hash_table_t.h>
#include <innodb/sync_rw/rw_lock_own.h>
#include <innodb/sync_rw/rw_lock_x_lock.h>

/** Reserves all the locks of a hash table, in an ascending order. */
void hash_lock_x_all(hash_table_t *table) /*!< in: hash table */
{
  ut_ad(table->type == HASH_TABLE_SYNC_RW_LOCK);

  for (ulint i = 0; i < table->n_sync_obj; i++) {
    rw_lock_t *lock = table->sync_obj.rw_locks + i;

    ut_ad(!rw_lock_own(lock, RW_LOCK_S));
    ut_ad(!rw_lock_own(lock, RW_LOCK_X));

    rw_lock_x_lock(lock);
  }
}

#endif
