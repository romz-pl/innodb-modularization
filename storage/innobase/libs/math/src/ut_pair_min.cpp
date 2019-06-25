#include <innodb/math/ut_pair_min.h>

#ifndef UNIV_HOTBACKUP
/** Calculate the minimum of two pairs.
@param[out]	min_hi	MSB of the minimum pair
@param[out]	min_lo	LSB of the minimum pair
@param[in]	a_hi	MSB of the first pair
@param[in]	a_lo	LSB of the first pair
@param[in]	b_hi	MSB of the second pair
@param[in]	b_lo	LSB of the second pair */
void ut_pair_min(ulint *min_hi, ulint *min_lo, ulint a_hi, ulint a_lo,
                 ulint b_hi, ulint b_lo) {
  if (a_hi == b_hi) {
    *min_hi = a_hi;
    *min_lo = std::min(a_lo, b_lo);
  } else if (a_hi < b_hi) {
    *min_hi = a_hi;
    *min_lo = a_lo;
  } else {
    *min_hi = b_hi;
    *min_lo = b_lo;
  }
}
#endif /* !UNIV_HOTBACKUP */

