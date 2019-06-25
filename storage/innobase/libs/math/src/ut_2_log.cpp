#include <innodb/math/ut_2_log.h>
#include <innodb/assert/assert.h>

/** Calculates fast the 2-logarithm of a number, rounded upward to an
 integer.
 @return logarithm in the base 2, rounded upward */
ulint ut_2_log(ulint n) /*!< in: number != 0 */
{
  ulint res;

  res = 0;

  ut_ad(n > 0);

  n = n - 1;

  for (;;) {
    n = n / 2;

    if (n == 0) {
      break;
    }

    res++;
  }

  return (res + 1);
}
