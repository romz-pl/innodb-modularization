#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_rw/rw_lock_sx_lock_low.h>
#include <innodb/sync_rw/pfs_rw_lock_sx_lock_low.h>

#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP
#ifndef UNIV_PFS_RWLOCK

#define rw_lock_sx_lock_nowait(M, P) \
  rw_lock_sx_lock_low((M), (P), __FILE__, __LINE__)

#else /* !UNIV_PFS_RWLOCK */

#define rw_lock_sx_lock_nowait(M, P) \
  pfs_rw_lock_sx_lock_low((M), (P), __FILE__, __LINE__)

#endif /* UNIV_PFS_RWLOCK */
#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */
