#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_rw/rw_lock_x_lock_wait_func.h>

#ifdef UNIV_DEBUG

#define rw_lock_x_lock_wait(L, P, T, F, O) \
  rw_lock_x_lock_wait_func(L, P, T, F, O)

#else

#define rw_lock_x_lock_wait(L, P, T, F, O) rw_lock_x_lock_wait_func(L, T, F, O)

#endif /* UNIV_DBEUG */
