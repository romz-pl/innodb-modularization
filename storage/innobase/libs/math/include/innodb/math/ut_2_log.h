#pragma once

#include <innodb/univ/univ.h>

/** Calculates fast the 2-logarithm of a number, rounded upward to an
 integer.
 @return logarithm in the base 2, rounded upward */
ulint ut_2_log(ulint n); /*!< in: number */
