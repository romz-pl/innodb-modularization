#include <innodb/align/ut_align_offset.h>
#include <innodb/assert/assert.h>

/** The following function computes the offset of a pointer from the nearest
 aligned address.
 @return distance from aligned pointer */
ulint ut_align_offset(const void *ptr, /*!< in: pointer */
                      ulint align_no)  /*!< in: align by this number */
{
  ut_ad(align_no > 0);
  ut_ad(((align_no - 1) & align_no) == 0);
  ut_ad(ptr);

  ut_ad(sizeof(void *) == sizeof(ulint));

  return (((ulint)ptr) & (align_no - 1));
}
