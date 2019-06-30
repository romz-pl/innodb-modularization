#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_rw/rw_lock_free_func.h>
#include <innodb/sync_rw/pfs_rw_lock_free_func.h>

#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP
#ifndef UNIV_PFS_RWLOCK

#define rw_lock_free(M) rw_lock_free_func(M)

#else /* !UNIV_PFS_RWLOCK */

#define rw_lock_free(M) pfs_rw_lock_free_func(M)

#endif /* UNIV_PFS_RWLOCK */
#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */
