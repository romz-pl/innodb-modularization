#include <innodb/record/rec_get_nth_field_offs_old.h>

#include <innodb/assert/assert.h>
#include <innodb/record/flag.h>
#include <innodb/record/rec_1_get_field_end_info.h>
#include <innodb/record/rec_1_get_field_start_offs.h>
#include <innodb/record/rec_2_get_field_start_offs.h>
#include <innodb/record/rec_get_1byte_offs_flag.h>
#include <innodb/record/rec_get_n_fields_old_raw.h>
#include <innodb/record/rec_2_get_field_end_info.h>

/** The following function is used to get the offset to the nth
 data field in an old-style record.
 @return offset to the field */
ulint rec_get_nth_field_offs_old(const rec_t *rec, /*!< in: record */
                                 ulint n,    /*!< in: index of the field */
                                 ulint *len) /*!< out: length of the field;
                                             UNIV_SQL_NULL if SQL null */
{
  ulint os;
  ulint next_os;

  ut_ad(len);
  ut_a(rec);
  ut_a(n < rec_get_n_fields_old_raw(rec));

  if (rec_get_1byte_offs_flag(rec)) {
    os = rec_1_get_field_start_offs(rec, n);

    next_os = rec_1_get_field_end_info(rec, n);

    if (next_os & REC_1BYTE_SQL_NULL_MASK) {
      *len = UNIV_SQL_NULL;

      return (os);
    }

    next_os = next_os & ~REC_1BYTE_SQL_NULL_MASK;
  } else {
    os = rec_2_get_field_start_offs(rec, n);

    next_os = rec_2_get_field_end_info(rec, n);

    if (next_os & REC_2BYTE_SQL_NULL_MASK) {
      *len = UNIV_SQL_NULL;

      return (os);
    }

    next_os = next_os & ~(REC_2BYTE_SQL_NULL_MASK | REC_2BYTE_EXTERN_MASK);
  }

  *len = next_os - os;

  ut_ad(*len < UNIV_PAGE_SIZE);

  return (os);
}

