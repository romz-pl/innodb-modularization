#include <innodb/bit/ut_bit_set_nth.h>
#include <innodb/assert/assert.h>

/** Sets the nth bit of a ulint.
 @return the ulint with the bit set as requested */
ulint ut_bit_set_nth(ulint a,   /*!< in: ulint */
                     ulint n,   /*!< in: nth bit requested */
                     ibool val) /*!< in: value for the bit to set */
{
  ut_ad(n < 8 * sizeof(ulint));
#if TRUE != 1
#error "TRUE != 1"
#endif
  if (val) {
    return (((ulint)1 << n) | a);
  } else {
    return (~((ulint)1 << n) & a);
  }
}
