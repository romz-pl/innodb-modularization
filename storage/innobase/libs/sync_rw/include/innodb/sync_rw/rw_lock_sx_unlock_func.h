#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory_check/memory_check.h>
#include <innodb/sync_rw/X_LOCK_HALF_DECR.h>
#include <innodb/sync_rw/rw_lock_reset_waiter_flag.h>
#include <innodb/sync_rw/rw_lock_lock_word_incr.h>
#include <innodb/error/ut_error.h>

#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP

/** Releases an sx mode lock.
@param[in]	pass	pass value; != 0, if the lock will be passed
                        to another thread to unlock
@param[in,out]	lock	rw-lock */
UNIV_INLINE
void rw_lock_sx_unlock_func(
#ifdef UNIV_DEBUG
    ulint pass,
#endif /* UNIV_DEBUG */
    rw_lock_t *lock);

#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */


#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP

/** Releases a sx mode lock. */
UNIV_INLINE
void rw_lock_sx_unlock_func(
#ifdef UNIV_DEBUG
    ulint pass,      /*!< in: pass value; != 0, if the lock may have
                     been passed to another thread to unlock */
#endif               /* UNIV_DEBUG */
    rw_lock_t *lock) /*!< in/out: rw-lock */
{
  ut_ad(rw_lock_get_sx_lock_count(lock));
  ut_ad(lock->sx_recursive > 0);

  --lock->sx_recursive;

  ut_d(rw_lock_remove_debug_info(lock, pass, RW_LOCK_SX));

  if (lock->sx_recursive == 0) {
    /* Last caller in a possible recursive chain. */
    if (lock->lock_word > 0) {
      lock->recursive = FALSE;
      UNIV_MEM_INVALID(&lock->writer_thread, sizeof lock->writer_thread);

      if (rw_lock_lock_word_incr(lock, X_LOCK_HALF_DECR) <= X_LOCK_HALF_DECR) {
        ut_error;
      }
      /* Lock is now free. May have to signal read/write
      waiters. We do not need to signal wait_ex waiters,
      since they cannot exist when there is an sx-lock
      holder. */
      if (lock->waiters) {
        rw_lock_reset_waiter_flag(lock);
        os_event_set(lock->event);
        sync_array_object_signalled();
      }
    } else {
      /* still has x-lock */
      ut_ad(lock->lock_word == -X_LOCK_HALF_DECR ||
            lock->lock_word <= -(X_LOCK_DECR + X_LOCK_HALF_DECR));
      lock->lock_word += X_LOCK_HALF_DECR;
    }
  }

  ut_ad(rw_lock_validate(lock));
}


#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */
