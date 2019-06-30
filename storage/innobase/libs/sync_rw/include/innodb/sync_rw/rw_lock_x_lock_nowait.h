#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP
#ifndef UNIV_PFS_RWLOCK

#define rw_lock_x_lock_nowait(M) \
  rw_lock_x_lock_func_nowait((M), __FILE__, __LINE__)

#else /* !UNIV_PFS_RWLOCK */

#define rw_lock_x_lock_nowait(M) \
  pfs_rw_lock_x_lock_func_nowait((M), __FILE__, __LINE__)

#endif /* UNIV_PFS_RWLOCK */
#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */
