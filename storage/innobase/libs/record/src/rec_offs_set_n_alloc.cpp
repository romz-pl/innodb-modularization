#include <innodb/record/rec_offs_set_n_alloc.h>

#include <innodb/record/flag.h>
#include <innodb/assert/assert.h>
#include <innodb/memory_check/memory_check.h>

/** The following function sets the number of allocated elements
 for an array of offsets. */
void rec_offs_set_n_alloc(ulint *offsets, /*!< out: array for rec_get_offsets(),
                                          must be allocated */
                          ulint n_alloc)  /*!< in: number of elements */
{
  ut_ad(offsets);
  ut_ad(n_alloc > REC_OFFS_HEADER_SIZE);
  UNIV_MEM_ASSERT_AND_ALLOC(offsets, n_alloc * sizeof *offsets);
  offsets[0] = n_alloc;
}
