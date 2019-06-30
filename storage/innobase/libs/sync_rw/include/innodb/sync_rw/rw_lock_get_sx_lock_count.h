#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_rw/rw_lock_t.h>
#include <innodb/sync_rw/X_LOCK_DECR.h>
#include <innodb/sync_rw/X_LOCK_HALF_DECR.h>

#ifndef UNIV_LIBRARY

/** Returns the number of sx-lock for the lock. Does not reserve the lock
 mutex, so the caller must be sure it is not changed during the call.
 @return value of writer_count */
UNIV_INLINE
ulint rw_lock_get_sx_lock_count(const rw_lock_t *lock); /*!< in: rw-lock */

#endif /* UNIV_LIBRARY */

#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP

/** Returns the number of sx-lock for the lock. Does not reserve the lock
 mutex, so the caller must be sure it is not changed during the call.
 @return value of sx-lock count */
UNIV_INLINE
ulint rw_lock_get_sx_lock_count(const rw_lock_t *lock) /*!< in: rw-lock */
{
#ifdef UNIV_DEBUG
  lint lock_copy = lock->lock_word;

  ut_ad(lock_copy <= X_LOCK_DECR);

  while (lock_copy < 0) {
    lock_copy += X_LOCK_DECR;
  }

  if (lock_copy > 0 && lock_copy <= X_LOCK_HALF_DECR) {
    return (lock->sx_recursive);
  }

  return (0);
#else  /* UNIV_DEBUG */
  return (lock->sx_recursive);
#endif /* UNIV_DEBUG */
}

#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */
