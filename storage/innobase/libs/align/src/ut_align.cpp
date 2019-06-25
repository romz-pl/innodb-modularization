#include <innodb/align/ut_align.h>
#include <innodb/assert/assert.h>


/** The following function rounds up a pointer to the nearest aligned address.
 @return aligned pointer */
void *ut_align(const void *ptr, /*!< in: pointer */
               ulint align_no)  /*!< in: align by this number */
{
  ut_ad(align_no > 0);
  ut_ad(((align_no - 1) & align_no) == 0);
  ut_ad(ptr);

  ut_ad(sizeof(void *) == sizeof(ulint));

  return ((void *)((((ulint)ptr) + align_no - 1) & ~(align_no - 1)));
}
