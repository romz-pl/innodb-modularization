#include <innodb/record/rec_set_next_offs_old.h>

#include <innodb/assert/assert.h>
#include <innodb/machine/data.h>
#include <innodb/record/flag.h>

/** The following function is used to set the next record offset field
 of an old-style record. */
void rec_set_next_offs_old(rec_t *rec, /*!< in: old-style physical record */
                           ulint next) /*!< in: offset of the next record */
{
  ut_ad(rec);
  ut_ad(UNIV_PAGE_SIZE > next);
#if REC_NEXT_MASK != 0xFFFFUL
#error "REC_NEXT_MASK != 0xFFFFUL"
#endif
#if REC_NEXT_SHIFT
#error "REC_NEXT_SHIFT != 0"
#endif

  mach_write_to_2(rec - REC_NEXT, next);
}
