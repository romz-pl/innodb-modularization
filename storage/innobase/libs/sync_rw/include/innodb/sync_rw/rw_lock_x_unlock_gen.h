#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_rw/rw_lock_x_unlock_func.h>
#include <innodb/sync_rw/pfs_rw_lock_x_unlock_func.h>

#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP
#ifndef UNIV_PFS_RWLOCK

    #ifdef UNIV_DEBUG
    #define rw_lock_x_unlock_gen(L, P) rw_lock_x_unlock_func(P, L)
    #else
    #define rw_lock_x_unlock_gen(L, P) rw_lock_x_unlock_func(L)
    #endif /* UNIV_DEBUG */

#else /* !UNIV_PFS_RWLOCK */

    #ifdef UNIV_DEBUG
        #define rw_lock_x_unlock_gen(L, P) pfs_rw_lock_x_unlock_func(P, L)
    #else
        #define rw_lock_x_unlock_gen(L, P) pfs_rw_lock_x_unlock_func(L)
    #endif

#endif /* UNIV_PFS_RWLOCK */
#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */
