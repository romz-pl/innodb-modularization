#include <innodb/sync_rw/rw_lock_validate.h>

#ifdef UNIV_DEBUG

/** Checks that the rw-lock has been initialized and that there are no
 simultaneous shared and exclusive locks.
 @return true */
bool rw_lock_validate(const rw_lock_t *lock) /*!< in: rw-lock */
{
  ulint waiters;
  lint lock_word;

  ut_ad(lock);

  waiters = rw_lock_get_waiters(lock);
  lock_word = lock->lock_word;

  ut_ad(lock->magic_n == RW_LOCK_MAGIC_N);
  ut_ad(waiters == 0 || waiters == 1);
  ut_ad(lock_word > -(2 * X_LOCK_DECR));
  ut_ad(lock_word <= X_LOCK_DECR);

  return (true);
}

#endif
