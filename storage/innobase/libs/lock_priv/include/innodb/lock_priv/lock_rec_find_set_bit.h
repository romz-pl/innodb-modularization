#pragma once

#include <innodb/univ/univ.h>

struct lock_t;

/** Looks for a set bit in a record lock bitmap. Returns ULINT_UNDEFINED,
 if none found.
 @return bit index == heap number of the record, or ULINT_UNDEFINED if
 none found */
ulint lock_rec_find_set_bit(
    const lock_t *lock); /*!< in: record lock with at least one
                         bit set */
