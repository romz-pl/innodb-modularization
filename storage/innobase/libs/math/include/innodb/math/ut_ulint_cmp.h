#pragma once

#include <innodb/univ/univ.h>

/** Compares two ulints.
@param[in]	a	ulint
@param[in]	b	ulint
@return 1 if a > b, 0 if a == b, -1 if a < b */
int ut_ulint_cmp(ulint a, ulint b);
