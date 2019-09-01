#pragma once

#include <innodb/univ/univ.h>

#include <innodb/lock_types/lock_mode.h>

struct lock_t;

/** Gets the mode of a lock.
 @return mode */
enum lock_mode lock_get_mode(const lock_t *lock); /*!< in: lock */
