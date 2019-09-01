#pragma once

#include <innodb/univ/univ.h>

struct lock_t;

/** Gets the wait flag of a lock.
 @return LOCK_WAIT if waiting, 0 if not */
ulint lock_get_wait(const lock_t *lock); /*!< in: lock */
