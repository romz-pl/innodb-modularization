#pragma once

#include <innodb/univ/univ.h>

struct lock_t;

/** Gets the type of a lock.
 @return LOCK_TABLE or LOCK_REC */
uint32_t lock_get_type_low(const lock_t *lock); /*!< in: lock */
