#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_rw/rw_lock_lock_word_incr.h>
#include <innodb/sync_rw/X_LOCK_HALF_DECR.h>

#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP

/** Releases a shared mode lock.
@param[in]	pass	pass value; != 0, if the lock will be passed
                        to another thread to unlock
@param[in,out]	lock	rw-lock */
UNIV_INLINE
void rw_lock_s_unlock_func(
#ifdef UNIV_DEBUG
    ulint pass,
#endif /* UNIV_DEBUG */
    rw_lock_t *lock);

#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */

#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP

/** Releases a shared mode lock. */
UNIV_INLINE
void rw_lock_s_unlock_func(
#ifdef UNIV_DEBUG
    ulint pass,      /*!< in: pass value; != 0, if the lock may have
                     been passed to another thread to unlock */
#endif               /* UNIV_DEBUG */
    rw_lock_t *lock) /*!< in/out: rw-lock */
{
  ut_ad(lock->lock_word > -X_LOCK_DECR);
  ut_ad(lock->lock_word != 0);
  ut_ad(lock->lock_word < X_LOCK_DECR);

  ut_d(rw_lock_remove_debug_info(lock, pass, RW_LOCK_S));

  /* Increment lock_word to indicate 1 less reader */
  lint lock_word = rw_lock_lock_word_incr(lock, 1);
  if (lock_word == 0 || lock_word == -X_LOCK_HALF_DECR) {
    /* wait_ex waiter exists. It may not be asleep, but we signal
    anyway. We do not wake other waiters, because they can't
    exist without wait_ex waiter and wait_ex waiter goes first.*/
    os_event_set(lock->wait_ex_event);
    sync_array_object_signalled();
  }

  ut_ad(rw_lock_validate(lock));
}

#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */
