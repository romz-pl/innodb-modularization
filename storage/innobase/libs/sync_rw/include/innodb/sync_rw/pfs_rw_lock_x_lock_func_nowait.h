#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_rw/rw_lock_t.h>
#include <innodb/sync_rw/rw_lock_x_lock_func_nowait.h>

#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP
#ifdef UNIV_PFS_RWLOCK

/** Performance schema instrumented wrap function for
rw_lock_x_lock_func_nowait()
NOTE! Please use the corresponding macro, not directly this function!
@param[in]	lock		pointer to rw-lock
@param[in]	file_name	file name where lock requested
@param[in]	line		line where requested
@return true if success */
UNIV_INLINE
ibool pfs_rw_lock_x_lock_func_nowait(rw_lock_t *lock, const char *file_name,
                                     ulint line);

#endif /* UNIV_PFS_RWLOCK */
#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */


#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP
#ifdef UNIV_PFS_RWLOCK

/** Performance schema instrumented wrap function for
 rw_lock_x_lock_func_nowait()
 NOTE! Please use the corresponding macro rw_lock_x_lock_func(),
 not directly this function!
 @return true if success */
UNIV_INLINE
ibool pfs_rw_lock_x_lock_func_nowait(
    rw_lock_t *lock,       /*!< in: pointer to rw-lock */
    const char *file_name, /*!< in: file name where lock
                         requested */
    ulint line)            /*!< in: line where requested */
{
  ibool ret;

  if (lock->pfs_psi != NULL) {
    PSI_rwlock_locker *locker;
    PSI_rwlock_locker_state state;

    /* Record the acquisition of a read-write trylock in exclusive
    mode in performance schema */

    locker = PSI_RWLOCK_CALL(start_rwlock_wrwait)(
        &state, lock->pfs_psi, PSI_RWLOCK_TRYEXCLUSIVELOCK, file_name,
        static_cast<uint>(line));

    ret = rw_lock_x_lock_func_nowait(lock, file_name, line);

    if (locker != NULL) {
      PSI_RWLOCK_CALL(end_rwlock_wrwait)(locker, static_cast<int>(ret));
    }
  } else {
    ret = rw_lock_x_lock_func_nowait(lock, file_name, line);
  }

  return (ret);
}

#endif /* UNIV_PFS_RWLOCK */
#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */
