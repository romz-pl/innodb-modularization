#include <innodb/hash/hash_unlock_x_all_but.h>


#ifndef UNIV_HOTBACKUP

#include <innodb/hash/hash_table_t.h>
#include <innodb/sync_rw/rw_lock_own.h>
#include <innodb/hash/hash_lock_x_all.h>
#include <innodb/sync_rw/rw_lock_x_unlock.h>

/** Releases all but passed in lock of a hash table, */
void hash_unlock_x_all_but(hash_table_t *table,  /*!< in: hash table */
                           rw_lock_t *keep_lock) /*!< in: lock to keep */
{
  ut_ad(table->type == HASH_TABLE_SYNC_RW_LOCK);

  for (ulint i = 0; i < table->n_sync_obj; i++) {
    rw_lock_t *lock = table->sync_obj.rw_locks + i;

    ut_ad(rw_lock_own(lock, RW_LOCK_X));

    if (keep_lock != lock) {
      rw_lock_x_unlock(lock);
    }
  }
}

#endif
