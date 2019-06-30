#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_rw/rw_lock_t.h>
#include <innodb/sync_rw/X_LOCK_DECR.h>
#include <innodb/sync_rw/X_LOCK_HALF_DECR.h>

#ifndef UNIV_LIBRARY

/** Returns the value of writer_count for the lock. Does not reserve the lock
 mutex, so the caller must be sure it is not changed during the call.
 @return value of writer_count */
UNIV_INLINE
ulint rw_lock_get_x_lock_count(const rw_lock_t *lock); /*!< in: rw-lock */

#endif /* UNIV_LIBRARY */



#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP

/** Returns the value of writer_count for the lock. Does not reserve the lock
 mutex, so the caller must be sure it is not changed during the call.
 @return value of writer_count */
UNIV_INLINE
ulint rw_lock_get_x_lock_count(const rw_lock_t *lock) /*!< in: rw-lock */
{
  lint lock_copy = lock->lock_word;
  ut_ad(lock_copy <= X_LOCK_DECR);

  if (lock_copy == 0 || lock_copy == -X_LOCK_HALF_DECR) {
    /* "1 x-lock" or "1 x-lock + sx-locks" */
    return (1);
  } else if (lock_copy > -X_LOCK_DECR) {
    /* s-locks, one or more sx-locks if > 0, or x-waiter if < 0 */
    return (0);
  } else if (lock_copy > -(X_LOCK_DECR + X_LOCK_HALF_DECR)) {
    /* no s-lock, no sx-lock, 2 or more x-locks.
    First 2 x-locks are set with -X_LOCK_DECR,
    all other recursive x-locks are set with -1 */
    return (2 - (lock_copy + X_LOCK_DECR));
  } else {
    /* no s-lock, 1 or more sx-lock, 2 or more x-locks.
    First 2 x-locks are set with -(X_LOCK_DECR + X_LOCK_HALF_DECR),
    all other recursive x-locks are set with -1 */
    return (2 - (lock_copy + X_LOCK_DECR + X_LOCK_HALF_DECR));
  }
}

#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */
