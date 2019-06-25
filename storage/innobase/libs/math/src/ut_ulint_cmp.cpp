#include <innodb/math/ut_ulint_cmp.h>

/** Compares two ulints.
 @return 1 if a > b, 0 if a == b, -1 if a < b */
int ut_ulint_cmp(ulint a, /*!< in: ulint */
                 ulint b) /*!< in: ulint */
{
  if (a < b) {
    return (-1);
  } else if (a == b) {
    return (0);
  } else {
    return (1);
  }
}
