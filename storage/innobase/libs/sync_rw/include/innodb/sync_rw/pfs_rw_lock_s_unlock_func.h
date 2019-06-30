#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_rw/rw_lock_t.h>
#include <innodb/sync_rw/rw_lock_s_unlock_func.h>

#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP
#ifdef UNIV_PFS_RWLOCK

/** Performance schema instrumented wrap function for rw_lock_s_unlock_func()
NOTE! Please use the corresponding macro rw_lock_s_unlock(), not directly this
function!
@param[in]	pass	pass value; != 0, if the lock may have been passed to
                        another thread to unlock
@param[in,out]	lock	rw-lock */
UNIV_INLINE
void pfs_rw_lock_s_unlock_func(
#ifdef UNIV_DEBUG
    ulint pass,
#endif /* UNIV_DEBUG */
    rw_lock_t *lock);

#endif /* UNIV_PFS_RWLOCK */
#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */


#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP
#ifdef UNIV_PFS_RWLOCK

/** Performance schema instrumented wrap function for rw_lock_s_unlock_func()
 NOTE! Please use the corresponding macro pfs_rw_lock_s_unlock(), not
 directly this function! */
ALWAYS_INLINE
void pfs_rw_lock_s_unlock_func(
#ifdef UNIV_DEBUG
    ulint pass,      /*!< in: pass value; != 0, if the
                     lock may have been passed to another
                     thread to unlock */
#endif               /* UNIV_DEBUG */
    rw_lock_t *lock) /*!< in/out: rw-lock */
{
  /* Inform performance schema we are unlocking the lock */
  if (lock->pfs_psi != NULL) {
    PSI_RWLOCK_CALL(unlock_rwlock)(lock->pfs_psi);
  }

  rw_lock_s_unlock_func(
#ifdef UNIV_DEBUG
      pass,
#endif /* UNIV_DEBUG */
      lock);
}

#endif /* UNIV_PFS_RWLOCK */
#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */
