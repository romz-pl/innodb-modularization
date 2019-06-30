#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_rw/rw_lock_t.h>
#include <innodb/sync_mutex/ib_mutex_t.h>

#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP
#ifndef INNODB_RW_LOCKS_USE_ATOMICS

UNIV_INLINE
ib_mutex_t *rw_lock_get_mutex(rw_lock_t *lock) {
    return (&(lock->mutex));
}

#endif /* INNODB_RW_LOCKS_USE_ATOMICS */
#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */
