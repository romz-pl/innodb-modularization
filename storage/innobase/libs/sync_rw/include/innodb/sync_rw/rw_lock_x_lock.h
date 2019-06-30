#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_rw/rw_lock_x_lock_func.h>
#include <innodb/sync_rw/pfs_rw_lock_x_lock_func.h>

#ifndef UNIV_HOTBACKUP
#ifdef UNIV_LIBRARY

#define rw_lock_x_lock(L) ((void)0)

#endif /* UNIV_LIBRARY */
#endif /* !UNIV_HOTBACKUP */


#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP
#ifndef UNIV_PFS_RWLOCK

#define rw_lock_x_lock(M) rw_lock_x_lock_func((M), 0, __FILE__, __LINE__)

#else /* !UNIV_PFS_RWLOCK */

#define rw_lock_x_lock(M) pfs_rw_lock_x_lock_func((M), 0, __FILE__, __LINE__)

#endif /* UNIV_PFS_RWLOCK */
#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */
