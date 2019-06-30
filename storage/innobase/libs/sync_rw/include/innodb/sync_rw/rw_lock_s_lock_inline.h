#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP
#ifdef UNIV_LIBRARY

#define rw_lock_s_lock_inline(M, P, F, L) ((void)0)

#endif /* UNIV_LIBRARY */
#endif /* !UNIV_HOTBACKUP */

#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP
#ifndef UNIV_PFS_RWLOCK

#define rw_lock_s_lock_inline(M, P, F, L) \
  rw_lock_s_lock_func((M), (P), (F), (L))

#else /* !UNIV_PFS_RWLOCK */

#define rw_lock_s_lock_inline(M, P, F, L) \
  pfs_rw_lock_s_lock_func((M), (P), (F), (L))

#endif /* UNIV_PFS_RWLOCK */
#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */
