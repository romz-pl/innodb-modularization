#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_rw/rw_lock_lock_word_decr.h>

#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP

/** Low-level function which tries to lock an rw-lock in s-mode. Performs no
spinning.
@param[in]	lock		pointer to rw-lock
@param[in]	pass		pass value; != 0, if the lock will be passed
                                to another thread to unlock
@param[in]	file_name	file name where lock requested
@param[in]	line		line where requested
@return true if success */
UNIV_INLINE
ibool rw_lock_s_lock_low(rw_lock_t *lock, ulint pass MY_ATTRIBUTE((unused)),
                         const char *file_name, ulint line);


#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */



#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP

/** Low-level function which tries to lock an rw-lock in s-mode. Performs no
 spinning.
 @return TRUE if success */
ALWAYS_INLINE
ibool rw_lock_s_lock_low(
    rw_lock_t *lock, /*!< in: pointer to rw-lock */
    ulint pass MY_ATTRIBUTE((unused)),
    /*!< in: pass value; != 0, if the lock will be
    passed to another thread to unlock */
    const char *file_name, /*!< in: file name where lock requested */
    ulint line)            /*!< in: line where requested */
{
  if (!rw_lock_lock_word_decr(lock, 1, 0)) {
    /* Locking did not succeed */
    return (FALSE);
  }

  ut_d(rw_lock_add_debug_info(lock, pass, RW_LOCK_S, file_name, line));

  /* These debugging values are not set safely: they may be incorrect
  or even refer to a line that is invalid for the file name. */
  lock->last_s_file_name = file_name;
  lock->last_s_line = line;

  return (TRUE); /* locking succeeded */
}

#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */
