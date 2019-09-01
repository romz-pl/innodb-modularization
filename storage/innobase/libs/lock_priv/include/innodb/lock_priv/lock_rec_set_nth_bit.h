#pragma once

#include <innodb/univ/univ.h>

struct lock_t;

/** Sets the nth bit of a record lock to TRUE.
@param[in]	lock	record lock
@param[in]	i	index of the bit */
UNIV_INLINE
void lock_rec_set_nth_bit(lock_t *lock, ulint i);
