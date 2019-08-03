#include <innodb/record/rec_offs_get_n_alloc.h>

#include <innodb/record/flag.h>
#include <innodb/assert/assert.h>
#include <innodb/memory_check/memory_check.h>

/** The following function returns the number of allocated elements
 for an array of offsets.
 @return number of elements */
MY_ATTRIBUTE((warn_unused_result)) ulint rec_offs_get_n_alloc(
    const ulint *offsets) /*!< in: array for rec_get_offsets() */
{
  ulint n_alloc;
  ut_ad(offsets);
  n_alloc = offsets[0];
  ut_ad(n_alloc > REC_OFFS_HEADER_SIZE);
  UNIV_MEM_ASSERT_W(offsets, n_alloc * sizeof *offsets);
  return (n_alloc);
}
