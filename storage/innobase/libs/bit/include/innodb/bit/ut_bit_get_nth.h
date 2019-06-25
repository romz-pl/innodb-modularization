#pragma once

#include <innodb/univ/univ.h>

/** Gets the nth bit of a ulint.
@param[in]	a	ulint
@param[in]	n	nth bit requested
@return true if nth bit is 1; 0th bit is defined to be the least significant */
ibool ut_bit_get_nth(ulint a, ulint n);
