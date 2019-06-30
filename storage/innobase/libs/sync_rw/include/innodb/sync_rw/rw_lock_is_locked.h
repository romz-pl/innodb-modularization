#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_rw/rw_lock_t.h>

#ifndef UNIV_LIBRARY

/** Checks if somebody has locked the rw-lock in the specified mode.
 @return true if locked */
bool rw_lock_is_locked(rw_lock_t *lock,  /*!< in: rw-lock */
                       ulint lock_type); /*!< in: lock type: RW_LOCK_S,
                                         RW_LOCK_X or RW_LOCK_SX */

#endif /* UNIV_LIBRARY */
