#pragma once

#include <innodb/univ/univ.h>

struct lock_t;

/** Gets the nth bit of a record lock.
@param[in]	lock	record lock
@param[in]	i	index of the bit
@return true if bit set also if i == ULINT_UNDEFINED return false */
bool lock_rec_get_nth_bit(const lock_t *lock, ulint i);
