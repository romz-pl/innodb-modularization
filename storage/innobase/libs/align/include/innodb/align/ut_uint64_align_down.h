#pragma once

#include <innodb/univ/univ.h>

/** Rounds a 64-bit integer downward to a multiple of a power of 2.
@param[in]	n		number to be rounded
@param[in]	align_no	align by this number
@return rounded value */
ib_uint64_t ut_uint64_align_down(ib_uint64_t n, ulint align_no);
