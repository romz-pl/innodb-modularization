#include <innodb/hash/hash_lock_x_confirm.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/sync_rw/rw_lock_own.h>
#include <innodb/hash/hash_get_lock.h>
#include <innodb/sync_rw/rw_lock_x_unlock.h>
#include <innodb/sync_rw/rw_lock_x_lock.h>

/** If not appropriate rw_lock for a fold value in a hash table,
relock X-lock the another rw_lock until appropriate for a fold value.
@param[in]	hash_lock	latched rw_lock to be confirmed
@param[in]	table		hash table
@param[in]	fold		fold value
@return	latched rw_lock */
rw_lock_t *hash_lock_x_confirm(rw_lock_t *hash_lock, hash_table_t *table,
                               ulint fold) {
  ut_ad(rw_lock_own(hash_lock, RW_LOCK_X));

  rw_lock_t *hash_lock_tmp = hash_get_lock(table, fold);

  while (hash_lock_tmp != hash_lock) {
    rw_lock_x_unlock(hash_lock);
    hash_lock = hash_lock_tmp;
    rw_lock_x_lock(hash_lock);
    hash_lock_tmp = hash_get_lock(table, fold);
  }

  return (hash_lock);
}

#endif
