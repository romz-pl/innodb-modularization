#pragma once

#include <innodb/univ/univ.h>

/** Calculates fast the remainder of n/m when m is a power of two.
 @param n in: numerator
 @param m in: denominator, must be a power of two
 @return the remainder of n/m */
#define ut_2pow_remainder(n, m) ((n) & ((m)-1))
