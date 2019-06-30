#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_rw/rw_lock_t.h>

#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP

/** Sets lock->waiters to 1. It is not an error if lock->waiters is already
 1. On platforms where ATOMIC builtins are used this function enforces a
 memory barrier. */
UNIV_INLINE
void rw_lock_set_waiter_flag(rw_lock_t *lock) /*!< in/out: rw-lock */
{
#ifdef INNODB_RW_LOCKS_USE_ATOMICS
  (void)os_compare_and_swap_ulint(&lock->waiters, 0, 1);
#else  /* INNODB_RW_LOCKS_USE_ATOMICS */
  lock->waiters = 1;
  os_wmb;
#endif /* INNODB_RW_LOCKS_USE_ATOMICS */
}

#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */
