#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_rw/rw_lock_t.h>
#include <innodb/sync_rw/X_LOCK_DECR.h>
#include <innodb/sync_rw/X_LOCK_HALF_DECR.h>

#ifndef UNIV_LIBRARY

/** Returns the write-status of the lock - this function made more sense
 with the old rw_lock implementation.
 @return RW_LOCK_NOT_LOCKED, RW_LOCK_X, RW_LOCK_X_WAIT, RW_LOCK_SX */
UNIV_INLINE
ulint rw_lock_get_writer(const rw_lock_t *lock); /*!< in: rw-lock */

#endif /* UNIV_LIBRARY */


#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP

/** Returns the write-status of the lock - this function made more sense
 with the old rw_lock implementation.
 @return RW_LOCK_NOT_LOCKED, RW_LOCK_X, RW_LOCK_X_WAIT, RW_LOCK_SX */
UNIV_INLINE
ulint rw_lock_get_writer(const rw_lock_t *lock) /*!< in: rw-lock */
{
  lint lock_word = lock->lock_word;

  ut_ad(lock_word <= X_LOCK_DECR);
  if (lock_word > X_LOCK_HALF_DECR) {
    /* return NOT_LOCKED in s-lock state, like the writer
    member of the old lock implementation. */
    return (RW_LOCK_NOT_LOCKED);
  } else if (lock_word > 0) {
    /* sx-locked, no x-locks */
    return (RW_LOCK_SX);
  } else if (lock_word == 0 || lock_word == -X_LOCK_HALF_DECR ||
             lock_word <= -X_LOCK_DECR) {
    /* x-lock with sx-lock is also treated as RW_LOCK_EX */
    return (RW_LOCK_X);
  } else {
    /* x-waiter with sx-lock is also treated as RW_LOCK_WAIT_EX
    e.g. -X_LOCK_HALF_DECR < lock_word < 0 : without sx
         -X_LOCK_DECR < lock_word < -X_LOCK_HALF_DECR : with sx */
    return (RW_LOCK_X_WAIT);
  }
}

#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */
