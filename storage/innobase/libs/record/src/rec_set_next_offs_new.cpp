#include <innodb/record/rec_set_next_offs_new.h>

#include <innodb/record/flag.h>
#include <innodb/assert/assert.h>
#include <innodb/machine/data.h>
#include <innodb/align/ut_align_offset.h>

/** The following function is used to set the next record offset field
 of a new-style record. */
void rec_set_next_offs_new(rec_t *rec, /*!< in/out: new-style physical record */
                           ulint next) /*!< in: offset of the next record */
{
  ulint field_value;

  ut_ad(rec);
  ut_ad(UNIV_PAGE_SIZE > next);

  if (!next) {
    field_value = 0;
  } else {
    /* The following two statements calculate
    next - offset_of_rec mod 64Ki, where mod is the modulo
    as a non-negative number */

    field_value =
        (ulint)((lint)next - (lint)ut_align_offset(rec, UNIV_PAGE_SIZE));
    field_value &= REC_NEXT_MASK;
  }

  mach_write_to_2(rec - REC_NEXT, field_value);
}
