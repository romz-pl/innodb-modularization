#pragma once

#include <innodb/univ/univ.h>

/** Calculates fast the number rounded up to the nearest power of 2.
 @return first power of 2 which is >= n */
ulint ut_2_power_up(ulint n) /*!< in: number != 0 */
    MY_ATTRIBUTE((const));
