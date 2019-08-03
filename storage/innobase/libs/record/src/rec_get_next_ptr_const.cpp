#include <innodb/record/rec_get_next_ptr_const.h>

#include <innodb/record/flag.h>
#include <innodb/assert/assert.h>
#include <innodb/align/ut_align_offset.h>
#include <innodb/align/ut_align_down.h>
#include <innodb/machine/data.h>

/** The following function is used to get the pointer of the next chained record
 on the same page.
 @return pointer to the next chained record, or NULL if none */
const rec_t *rec_get_next_ptr_const(
    const rec_t *rec, /*!< in: physical record */
    ulint comp)       /*!< in: nonzero=compact page format */
{
  ulint field_value;

  ut_ad(REC_NEXT_MASK == 0xFFFFUL);
  ut_ad(REC_NEXT_SHIFT == 0);

  field_value = mach_read_from_2(rec - REC_NEXT);

  if (field_value == 0) {
    return (NULL);
  }

  if (comp) {
    if (UNIV_PAGE_SIZE_MAX <= 32768) {
      /* Note that for 64 KiB pages, field_value can
      'wrap around' and the debug assertion is not valid */

      /* In the following assertion, field_value is
      interpreted as signed 16-bit integer in 2's complement
      arithmetics. If all platforms defined int16_t in the
      standard headers, the expression could be written
      simpler as:

      (int16_t) field_value + ut_align_offset(...)
      < UNIV_PAGE_SIZE */

      ut_ad((field_value >= 32768 ? field_value - 65536 : field_value) +
                ut_align_offset(rec, UNIV_PAGE_SIZE) <
            UNIV_PAGE_SIZE);
    }

    /* There must be at least REC_N_NEW_EXTRA_BYTES + 1
    between each record. */
    ut_ad((field_value > REC_N_NEW_EXTRA_BYTES && field_value < 32768) ||
          field_value < (uint16)-REC_N_NEW_EXTRA_BYTES);

    return ((byte *)ut_align_down(rec, UNIV_PAGE_SIZE) +
            ut_align_offset(rec + field_value, UNIV_PAGE_SIZE));
  } else {
    ut_ad(field_value < UNIV_PAGE_SIZE);

    return ((byte *)ut_align_down(rec, UNIV_PAGE_SIZE) + field_value);
  }
}
