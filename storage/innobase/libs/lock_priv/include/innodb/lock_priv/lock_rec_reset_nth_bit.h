#pragma once

#include <innodb/univ/univ.h>

struct lock_t;

/** Reset the nth bit of a record lock.
@param[in,out] lock record lock
@param[in] i index of the bit that will be reset
@return previous value of the bit */
UNIV_INLINE
byte lock_rec_reset_nth_bit(lock_t *lock, ulint i);
