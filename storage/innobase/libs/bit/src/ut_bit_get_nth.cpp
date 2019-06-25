#include <innodb/bit/ut_bit_get_nth.h>
#include <innodb/assert/assert.h>

/** Gets the nth bit of a ulint.
 @return true if nth bit is 1; 0th bit is defined to be the least significant */
ibool ut_bit_get_nth(ulint a, /*!< in: ulint */
                     ulint n) /*!< in: nth bit requested */
{
  ut_ad(n < 8 * sizeof(ulint));
#if TRUE != 1
#error "TRUE != 1"
#endif
  return (1 & (a >> n));
}
