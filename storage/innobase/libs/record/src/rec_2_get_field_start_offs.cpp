#include <innodb/record/rec_2_get_field_start_offs.h>

#include <innodb/record/rec_get_1byte_offs_flag.h>
#include <innodb/record/rec_get_n_fields_old_raw.h>
#include <innodb/record/rec_2_get_prev_field_end_info.h>
#include <innodb/record/flag.h>
#include <innodb/assert/assert.h>

/** Returns the offset of nth field start if the record is stored in the 2-byte
 offsets form.
 @return offset of the start of the field */
ulint rec_2_get_field_start_offs(const rec_t *rec, /*!< in: record */
                                 ulint n)          /*!< in: field index */
{
  ut_ad(!rec_get_1byte_offs_flag(rec));
  ut_ad(n <= rec_get_n_fields_old_raw(rec));

  if (n == 0) {
    return (0);
  }

  return (rec_2_get_prev_field_end_info(rec, n) &
          ~(REC_2BYTE_SQL_NULL_MASK | REC_2BYTE_EXTERN_MASK));
}
