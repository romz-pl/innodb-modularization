#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_rw/rw_lock_x_lock_func_nowait.h>
#include <innodb/sync_rw/pfs_rw_lock_x_lock_func_nowait.h>

#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP
#ifndef UNIV_PFS_RWLOCK

#define rw_lock_x_lock_func_nowait_inline(M, F, L) \
  rw_lock_x_lock_func_nowait((M), (F), (L))

#else /* !UNIV_PFS_RWLOCK */

#define rw_lock_x_lock_func_nowait_inline(M, F, L) \
  pfs_rw_lock_x_lock_func_nowait((M), (F), (L))

#endif /* UNIV_PFS_RWLOCK */
#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */
