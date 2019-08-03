#include <innodb/record/rec_get_next_offs.h>

#include <innodb/record/flag.h>
#include <innodb/assert/assert.h>
#include <innodb/align/ut_align_offset.h>
#include <innodb/machine/data.h>

/** The following function is used to get the offset of the next chained record
 on the same page.
 @return the page offset of the next chained record, or 0 if none */
ulint rec_get_next_offs(const rec_t *rec, /*!< in: physical record */
                        ulint comp) /*!< in: nonzero=compact page format */
{
  ulint field_value;
#if REC_NEXT_MASK != 0xFFFFUL
#error "REC_NEXT_MASK != 0xFFFFUL"
#endif
#if REC_NEXT_SHIFT
#error "REC_NEXT_SHIFT != 0"
#endif

  field_value = mach_read_from_2(rec - REC_NEXT);

  if (comp) {
    if (UNIV_PAGE_SIZE_MAX <= 32768) {
      /* Note that for 64 KiB pages, field_value can
      'wrap around' and the debug assertion is not valid */

      /* In the following assertion, field_value is
      interpreted as signed 16-bit integer in 2's complement
      arithmetics.

      If all platforms defined int16_t in the standard
      headers, the expression could be written simpler as

      (int16_t) field_value + ut_align_offset(...)
      < UNIV_PAGE_SIZE */

      ut_ad((field_value >= 32768 ? field_value - 65536 : field_value) +
                ut_align_offset(rec, UNIV_PAGE_SIZE) <
            UNIV_PAGE_SIZE);
    }

    if (field_value == 0) {
      return (0);
    }

    /* There must be at least REC_N_NEW_EXTRA_BYTES + 1
    between each record. */
    ut_ad((field_value > REC_N_NEW_EXTRA_BYTES && field_value < 32768) ||
          field_value < (uint16)-REC_N_NEW_EXTRA_BYTES);

    return (ut_align_offset(rec + field_value, UNIV_PAGE_SIZE));
  } else {
    ut_ad(field_value < UNIV_PAGE_SIZE);

    return (field_value);
  }
}
