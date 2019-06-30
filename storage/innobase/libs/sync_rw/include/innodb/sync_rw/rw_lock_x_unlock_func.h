#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_rw/X_LOCK_HALF_DECR.h>
#include <innodb/sync_rw/X_LOCK_DECR.h>
#include <innodb/sync_rw/rw_lock_lock_word_incr.h>
#include <innodb/error/ut_error.h>
#include <innodb/sync_rw/rw_lock_reset_waiter_flag.h>

#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP

/** Releases an exclusive mode lock.
@param[in]	pass	pass value; != 0, if the lock will be passed
                        to another thread to unlock
@param[in,out]	lock	rw-lock */
UNIV_INLINE
void rw_lock_x_unlock_func(
#ifdef UNIV_DEBUG
    ulint pass,
#endif /* UNIV_DEBUG */
    rw_lock_t *lock);

#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */


#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP

/** Releases an exclusive mode lock. */
UNIV_INLINE
void rw_lock_x_unlock_func(
#ifdef UNIV_DEBUG
    ulint pass,      /*!< in: pass value; != 0, if the lock may have
                     been passed to another thread to unlock */
#endif               /* UNIV_DEBUG */
    rw_lock_t *lock) /*!< in/out: rw-lock */
{
  ut_ad(lock->lock_word == 0 || lock->lock_word == -X_LOCK_HALF_DECR ||
        lock->lock_word <= -X_LOCK_DECR);

  /* lock->recursive flag also indicates if lock->writer_thread is
  valid or stale. If we are the last of the recursive callers
  then we must unset lock->recursive flag to indicate that the
  lock->writer_thread is now stale.
  Note that since we still hold the x-lock we can safely read the
  lock_word. */
  if (lock->lock_word == 0) {
    /* Last caller in a possible recursive chain. */
    lock->recursive = FALSE;
  }

  ut_d(rw_lock_remove_debug_info(lock, pass, RW_LOCK_X));

  if (lock->lock_word == 0 || lock->lock_word == -X_LOCK_HALF_DECR) {
    /* There is 1 x-lock */
    /* atomic increment is needed, because it is last */
    if (rw_lock_lock_word_incr(lock, X_LOCK_DECR) <= 0) {
      ut_error;
    }

    /* This no longer has an X-lock but it may still have
    an SX-lock. So it is now free for S-locks by other threads.
    We need to signal read/write waiters.
    We do not need to signal wait_ex waiters, since they cannot
    exist when there is a writer. */
    if (lock->waiters) {
      rw_lock_reset_waiter_flag(lock);
      os_event_set(lock->event);
      sync_array_object_signalled();
    }
  } else if (lock->lock_word == -X_LOCK_DECR ||
             lock->lock_word == -(X_LOCK_DECR + X_LOCK_HALF_DECR)) {
    /* There are 2 x-locks */
    lock->lock_word += X_LOCK_DECR;
  } else {
    /* There are more than 2 x-locks. */
    ut_ad(lock->lock_word < -X_LOCK_DECR);
    lock->lock_word += 1;
  }

  ut_ad(rw_lock_validate(lock));
}


#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */
