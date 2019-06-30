#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_rw/rw_lock_t.h>
#include <innodb/sync_rw/rw_lock_sx_lock_low.h>

#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP
#ifdef UNIV_PFS_RWLOCK

/** Performance schema instrumented wrap function for rw_lock_sx_lock_nowait()
NOTE! Please use the corresponding macro, not directly this function!
@param[in]	lock		pointer to rw-lock
@param[in]	pass		pass value; != 0, if the lock will be passed
                                to another thread to unlock
@param[in]	file_name	file name where lock requested
@param[in]	line		line where requested */
UNIV_INLINE
ibool pfs_rw_lock_sx_lock_low(rw_lock_t *lock, ulint pass,
                              const char *file_name, ulint line);

#endif /* UNIV_PFS_RWLOCK */
#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */

#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP
#ifdef UNIV_PFS_RWLOCK

/** Performance schema instrumented wrap function for rw_lock_sx_lock_nowait()
 NOTE! Please use the corresponding macro, not
 directly this function!
 @return true if success */
UNIV_INLINE
ibool pfs_rw_lock_sx_lock_low(
    rw_lock_t *lock,       /*!< in: pointer to rw-lock */
    ulint pass,            /*!< in: pass value; != 0, if the
                           lock will be passed to another
                           thread to unlock */
    const char *file_name, /*!< in: file name where lock requested */
    ulint line)            /*!< in: line where requested */
{
  ibool ret;

  if (lock->pfs_psi != NULL) {
    PSI_rwlock_locker *locker;
    PSI_rwlock_locker_state state;

    /* Instrumented to inform we are aquiring a shared
    exclusive rwlock */
    locker = PSI_RWLOCK_CALL(start_rwlock_rdwait)(
        &state, lock->pfs_psi, PSI_RWLOCK_TRYSHAREDEXCLUSIVELOCK, file_name,
        static_cast<uint>(line));

    ret = rw_lock_sx_lock_low(lock, pass, file_name, line);

    if (locker != NULL) {
      PSI_RWLOCK_CALL(end_rwlock_rdwait)(locker, static_cast<int>(ret));
    }
  } else {
    ret = rw_lock_sx_lock_low(lock, pass, file_name, line);
  }

  return (ret);
}

#endif /* UNIV_PFS_RWLOCK */
#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */
