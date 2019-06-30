#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_rw/rw_lock_s_lock_low.h>
#include <innodb/sync_rw/rw_lock_s_lock_spin.h>

#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP

/** NOTE! Use the corresponding macro, not directly this function, except if
you supply the file name and line number. Lock an rw-lock in shared mode for
the current thread. If the rw-lock is locked in exclusive mode, or there is an
exclusive lock request waiting, the function spins a preset time (controlled
by srv_n_spin_wait_rounds), waiting for the lock, before suspending the thread.
@param[in]	lock		pointer to rw-lock
@param[in]	pass		pass value; != 0, if the lock will be passed
                                to another thread to unlock
@param[in]	file_name	file name where lock requested
@param[in]	line		line where requested */
UNIV_INLINE
void rw_lock_s_lock_func(rw_lock_t *lock, ulint pass, const char *file_name,
                         ulint line);

#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */


#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP

/** NOTE! Use the corresponding macro, not directly this function! Lock an
 rw-lock in shared mode for the current thread. If the rw-lock is locked
 in exclusive mode, or there is an exclusive lock request waiting, the
 function spins a preset time (controlled by srv_n_spin_wait_rounds), waiting
 for the lock, before suspending the thread. */
UNIV_INLINE
void rw_lock_s_lock_func(
    rw_lock_t *lock,       /*!< in: pointer to rw-lock */
    ulint pass,            /*!< in: pass value; != 0, if the lock will
                           be passed to another thread to unlock */
    const char *file_name, /*!< in: file name where lock requested */
    ulint line)            /*!< in: line where requested */
{
  /* NOTE: As we do not know the thread ids for threads which have
  s-locked a latch, and s-lockers will be served only after waiting
  x-lock requests have been fulfilled, then if this thread already
  owns an s-lock here, it may end up in a deadlock with another thread
  which requests an x-lock here. Therefore, we will forbid recursive
  s-locking of a latch: the following assert will warn the programmer
  of the possibility of this kind of a deadlock. If we want to implement
  safe recursive s-locking, we should keep in a list the thread ids of
  the threads which have s-locked a latch. This would use some CPU
  time. */

  ut_ad(!rw_lock_own(lock, RW_LOCK_S)); /* see NOTE above */
  ut_ad(!rw_lock_own(lock, RW_LOCK_X));

  if (!rw_lock_s_lock_low(lock, pass, file_name, line)) {
    /* Did not succeed, try spin wait */

    rw_lock_s_lock_spin(lock, pass, file_name, line);
  }
}

#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */
