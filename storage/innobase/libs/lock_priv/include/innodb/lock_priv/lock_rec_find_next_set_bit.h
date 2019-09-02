#pragma once

#include <innodb/univ/univ.h>

struct lock_t;

/** Looks for the next set bit in the record lock bitmap.
@param[in] lock		record lock with at least one bit set
@param[in] heap_no	current set bit
@return The next bit index  == heap number following heap_no, or ULINT_UNDEFINED
if none found */
ulint lock_rec_find_next_set_bit(const lock_t *lock, ulint heap_no);
