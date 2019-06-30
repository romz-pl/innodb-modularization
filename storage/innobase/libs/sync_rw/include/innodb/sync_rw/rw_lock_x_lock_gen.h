#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP
#ifndef UNIV_PFS_RWLOCK

#define rw_lock_x_lock_gen(M, P) \
  rw_lock_x_lock_func((M), (P), __FILE__, __LINE__)

#else /* !UNIV_PFS_RWLOCK */

#define rw_lock_x_lock_gen(M, P) \
  pfs_rw_lock_x_lock_func((M), (P), __FILE__, __LINE__)

#endif /* UNIV_PFS_RWLOCK */
#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */
