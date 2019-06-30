#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_rw/rw_lock_sx_unlock_func.h>
#include <innodb/sync_rw/pfs_rw_lock_sx_unlock_func.h>

#ifndef UNIV_HOTBACKUP
#ifdef UNIV_LIBRARY

#define rw_lock_sx_unlock(L) ((void)0)

#endif /* UNIV_LIBRARY */
#endif /* !UNIV_HOTBACKUP */


#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP
#ifndef UNIV_PFS_RWLOCK

    #ifdef UNIV_DEBUG
        #define rw_lock_sx_unlock(L) rw_lock_sx_unlock_func(0, L)
    #else /* UNIV_DEBUG */
        #define rw_lock_sx_unlock(L) rw_lock_sx_unlock_func(L)
    #endif /* UNIV_DEBUG */

#else /* !UNIV_PFS_RWLOCK */

    #ifdef UNIV_DEBUG
        #define rw_lock_sx_unlock(L) pfs_rw_lock_sx_unlock_func(0, L)
    #else
        #define rw_lock_sx_unlock(L) pfs_rw_lock_sx_unlock_func(L)
    #endif

#endif /* UNIV_PFS_RWLOCK */
#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */
