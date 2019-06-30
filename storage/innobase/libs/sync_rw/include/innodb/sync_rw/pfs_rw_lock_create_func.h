#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_rw/rw_lock_t.h>
#include <innodb/sync_rw/rw_lock_create_func.h>

#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP
#ifdef UNIV_PFS_RWLOCK

/** Performance schema instrumented wrap function for rw_lock_create_func()
NOTE! Please use the corresponding macro rw_lock_create(), not directly this
function!
@param[in]	key		key registered with performance schema
@param[in]	lock		rw lock
@param[in]	level		level
@param[in]	cmutex_name	mutex name
@param[in]	cline		file line where created
@param[in]	cfile_name	file name where created */
UNIV_INLINE
void pfs_rw_lock_create_func(mysql_pfs_key_t key, rw_lock_t *lock,
#ifdef UNIV_DEBUG
                             latch_level_t level, const char *cmutex_name,
#endif /* UNIV_DEBUG */
                             const char *cfile_name, ulint cline);

#endif /* UNIV_PFS_RWLOCK */
#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */



#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP
#ifdef UNIV_PFS_RWLOCK

/** Performance schema instrumented wrap function for rw_lock_create_func().
 NOTE! Please use the corresponding macro rw_lock_create(), not directly
 this function! */
UNIV_INLINE
void pfs_rw_lock_create_func(
    mysql_pfs_key_t key, /*!< in: key registered with
                         performance schema */
    rw_lock_t *lock,     /*!< in/out: pointer to memory */
#ifdef UNIV_DEBUG
    latch_level_t level,     /*!< in: level */
    const char *cmutex_name, /*!< in: mutex name */
#endif                       /* UNIV_DEBUG */
    const char *cfile_name,  /*!< in: file name where created */
    ulint cline)             /*!< in: file line where created */
{
  ut_d(new (lock) rw_lock_t());

  /* Initialize the rwlock for performance schema */
  lock->pfs_psi = PSI_RWLOCK_CALL(init_rwlock)(key.m_value, lock);

  /* The actual function to initialize an rwlock */
  rw_lock_create_func(lock,
#ifdef UNIV_DEBUG
                      level, cmutex_name,
#endif /* UNIV_DEBUG */
                      cfile_name, cline);
}

#endif /* UNIV_PFS_RWLOCK */
#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */
