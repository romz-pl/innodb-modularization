#include <innodb/lock_priv/lock_rec_lock_fold.h>

#include <innodb/lock_priv/lock_t.h>
#include <innodb/lock_priv/lock_rec_fold.h>

/** Calculates the fold value of a lock: used in migrating the hash table.
@param[in]	lock	record lock object
@return	folded value */
ulint lock_rec_lock_fold(const lock_t *lock) {
  return (lock_rec_fold(lock->rec_lock.space, lock->rec_lock.page_no));
}
