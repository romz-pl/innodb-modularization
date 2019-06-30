#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_rw/rw_lock_s_lock_low.h>
#include <innodb/sync_rw/pfs_rw_lock_s_lock_low.h>

#ifndef UNIV_HOTBACKUP
#ifdef UNIV_LIBRARY

#define rw_lock_s_lock_nowait(M, F, L) true

#endif /* UNIV_LIBRARY */
#endif /* !UNIV_HOTBACKUP */


#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP
#ifndef UNIV_PFS_RWLOCK

#define rw_lock_s_lock_nowait(M, F, L) rw_lock_s_lock_low((M), 0, (F), (L))

#else /* !UNIV_PFS_RWLOCK */

#define rw_lock_s_lock_nowait(M, F, L) pfs_rw_lock_s_lock_low((M), 0, (F), (L))

#endif /* UNIV_PFS_RWLOCK */
#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */
