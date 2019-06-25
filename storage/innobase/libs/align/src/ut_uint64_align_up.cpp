#include <innodb/align/ut_uint64_align_up.h>
#include <innodb/assert/assert.h>
#include <innodb/math/ut_is_2pow.h>

/** Rounds ib_uint64_t upward to a multiple of a power of 2.
 @return rounded value */
ib_uint64_t ut_uint64_align_up(ib_uint64_t n,  /*!< in: number to be rounded */
                               ulint align_no) /*!< in: align by this number
                                               which must be a power of 2 */
{
  ib_uint64_t align_1 = (ib_uint64_t)align_no - 1;

  ut_ad(align_no > 0);
  ut_ad(ut_is_2pow(align_no));

  return ((n + align_1) & ~align_1);
}
