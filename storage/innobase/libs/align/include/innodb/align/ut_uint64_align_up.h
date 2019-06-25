#pragma once

#include <innodb/univ/univ.h>

/** Rounds ib_uint64_t upward to a multiple of a power of 2.
@param[in]	n		number to be rounded
@param[in]	align_no	align by this number
@return rounded value */
ib_uint64_t ut_uint64_align_up(ib_uint64_t n, ulint align_no);
