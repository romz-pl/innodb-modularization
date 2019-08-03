#include <innodb/record/rec_get_field_start_offs.h>

#include <innodb/record/rec_get_n_fields_old_raw.h>
#include <innodb/record/rec_get_1byte_offs_flag.h>
#include <innodb/record/rec_1_get_field_start_offs.h>
#include <innodb/record/rec_2_get_field_start_offs.h>
#include <innodb/assert/assert.h>

/** The following function is used to read the offset of the start of a data
 field in the record. The start of an SQL null field is the end offset of the
 previous non-null field, or 0, if none exists. If n is the number of the last
 field + 1, then the end offset of the last field is returned.
 @return offset of the start of the field */
ulint rec_get_field_start_offs(const rec_t *rec, /*!< in: record */
                               ulint n)          /*!< in: field index */
{
  ut_ad(rec);
  ut_ad(n <= rec_get_n_fields_old_raw(rec));

  if (n == 0) {
    return (0);
  }

  if (rec_get_1byte_offs_flag(rec)) {
    return (rec_1_get_field_start_offs(rec, n));
  }

  return (rec_2_get_field_start_offs(rec, n));
}
