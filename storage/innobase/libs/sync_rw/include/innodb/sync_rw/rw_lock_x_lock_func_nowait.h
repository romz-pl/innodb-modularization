#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_rw/X_LOCK_HALF_DECR.h>
#include <innodb/sync_rw/X_LOCK_DECR.h>
#include <innodb/sync_rw/rw_lock_set_writer_id_and_recursion_flag.h>
#include <innodb/thread/os_thread_eq.h>
#include <innodb/thread/os_thread_get_curr_id.h>

#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP

/** NOTE! Use the corresponding macro, not directly this function! Lock an
rw-lock in exclusive mode for the current thread if the lock can be obtained
immediately.
@param[in]	lock		pointer to rw-lock
@param[in]	file_name	file name where lock requested
@param[in]	line		line where requested
@return true if success */
UNIV_INLINE
ibool rw_lock_x_lock_func_nowait(rw_lock_t *lock, const char *file_name,
                                 ulint line);

#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */

#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP

/** NOTE! Use the corresponding macro, not directly this function! Lock an
 rw-lock in exclusive mode for the current thread if the lock can be
 obtained immediately.
 @return true if success */
UNIV_INLINE
ibool rw_lock_x_lock_func_nowait(
    rw_lock_t *lock,       /*!< in: pointer to rw-lock */
    const char *file_name, /*!< in: file name where lock requested */
    ulint line)            /*!< in: line where requested */
{
  ibool success;

#ifdef INNODB_RW_LOCKS_USE_ATOMICS
  success = os_compare_and_swap_lint(&lock->lock_word, X_LOCK_DECR, 0);
#else

  success = FALSE;
  mutex_enter(&(lock->mutex));
  if (lock->lock_word == X_LOCK_DECR) {
    lock->lock_word = 0;
    success = TRUE;
  }
  mutex_exit(&(lock->mutex));

#endif
  if (success) {
    rw_lock_set_writer_id_and_recursion_flag(lock, true);

  } else if (lock->recursive &&
             os_thread_eq(lock->writer_thread, os_thread_get_curr_id())) {
    /* Relock: this lock_word modification is safe since no other
    threads can modify (lock, unlock, or reserve) lock_word while
    there is an exclusive writer and this is the writer thread. */
    if (lock->lock_word == 0 || lock->lock_word == -X_LOCK_HALF_DECR) {
      /* There are 1 x-locks */
      lock->lock_word -= X_LOCK_DECR;
    } else if (lock->lock_word <= -X_LOCK_DECR) {
      /* There are 2 or more x-locks */
      lock->lock_word--;
    } else {
      /* Failure */
      return (FALSE);
    }

    /* Watch for too many recursive locks */
    ut_ad(lock->lock_word < 0);

  } else {
    /* Failure */
    return (FALSE);
  }

  ut_d(rw_lock_add_debug_info(lock, 0, RW_LOCK_X, file_name, line));

  lock->last_x_file_name = file_name;
  lock->last_x_line = line;

  ut_ad(rw_lock_validate(lock));

  return (TRUE);
}


#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */
