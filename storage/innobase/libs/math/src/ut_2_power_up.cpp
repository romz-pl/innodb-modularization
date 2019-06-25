#include <innodb/math/ut_2_power_up.h>
#include <innodb/assert/assert.h>

/** Calculates fast the number rounded up to the nearest power of 2.
 @return first power of 2 which is >= n */
ulint ut_2_power_up(ulint n) /*!< in: number != 0 */
{
  ulint res;

  res = 1;

  ut_ad(n > 0);

  while (res < n) {
    res = res * 2;
  }

  return (res);
}
