#include <innodb/lock_priv/lock_get_wait.h>

#include <innodb/lock_priv/lock_t.h>

/** Gets the wait flag of a lock.
 @return LOCK_WAIT if waiting, 0 if not */
ulint lock_get_wait(const lock_t *lock) /*!< in: lock */
{
  ut_ad(lock);

  return (lock->type_mode & LOCK_WAIT);
}
