#pragma once

#include <innodb/univ/univ.h>

struct lock_t;

/** Gets the number of bits in a record lock bitmap.
 @return number of bits */
ulint lock_rec_get_n_bits(const lock_t *lock); /*!< in: record lock */
