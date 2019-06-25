#pragma once

#include <innodb/univ/univ.h>

/** Sets the nth bit of a ulint.
@param[in]	a	ulint
@param[in]	n	nth bit requested
@param[in]	val	value for the bit to set
@return the ulint with the bit set as requested */
ulint ut_bit_set_nth(ulint a, ulint n, ibool val);
