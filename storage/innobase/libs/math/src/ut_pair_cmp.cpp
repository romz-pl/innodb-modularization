#include <innodb/math/ut_pair_cmp.h>
#include <innodb/math/ut_ulint_cmp.h>

/** Compare two pairs of integers.
@param[in]	a_h	more significant part of first pair
@param[in]	a_l	less significant part of first pair
@param[in]	b_h	more significant part of second pair
@param[in]	b_l	less significant part of second pair
@return comparison result of (a_h,a_l) and (b_h,b_l)
@retval -1 if (a_h,a_l) is less than (b_h,b_l)
@retval 0 if (a_h,a_l) is equal to (b_h,b_l)
@retval 1 if (a_h,a_l) is greater than (b_h,b_l) */
int ut_pair_cmp(ulint a_h, ulint a_l, ulint b_h, ulint b_l) {
  if (a_h < b_h) {
    return (-1);
  }
  if (a_h > b_h) {
    return (1);
  }
  return (ut_ulint_cmp(a_l, b_l));
}
