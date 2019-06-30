#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_rw/rw_lock_t.h>
#include <innodb/sync_rw/rw_lock_free_func.h>

#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP
#ifdef UNIV_PFS_RWLOCK

/** Performance schema instrumented wrap function for rw_lock_free_func()
 NOTE! Please use the corresponding macro rw_lock_free(), not directly
 this function! */
UNIV_INLINE
void pfs_rw_lock_free_func(rw_lock_t *lock); /*!< in: rw-lock */

#endif /* UNIV_PFS_RWLOCK */
#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */


#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP
#ifdef UNIV_PFS_RWLOCK

/** Performance schema instrumented wrap function for rw_lock_free_func()
 NOTE! Please use the corresponding macro rw_lock_free(), not directly
 this function! */
UNIV_INLINE
void pfs_rw_lock_free_func(rw_lock_t *lock) /*!< in: pointer to rw-lock */
{
  if (lock->pfs_psi != NULL) {
    PSI_RWLOCK_CALL(destroy_rwlock)(lock->pfs_psi);
    lock->pfs_psi = NULL;
  }

  rw_lock_free_func(lock);
}

#endif /* UNIV_PFS_RWLOCK */
#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */
