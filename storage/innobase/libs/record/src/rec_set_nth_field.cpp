#include <innodb/record/rec_set_nth_field.h>

#include <innodb/assert/assert.h>
#include <innodb/memory/ut_memcpy.h>
#include <innodb/record/rec_get_nth_field.h>
#include <innodb/record/rec_get_nth_field_size.h>
#include <innodb/record/rec_offs_comp.h>
#include <innodb/record/rec_offs_nth_default.h>
#include <innodb/record/rec_offs_nth_sql_null.h>
#include <innodb/record/rec_offs_validate.h>
#include <innodb/record/rec_set_nth_field_null_bit.h>
#include <innodb/record/rec_set_nth_field_sql_null.h>

/** This is used to modify the value of an already existing field in a record.
 The previous value must have exactly the same size as the new value. If len
 is UNIV_SQL_NULL then the field is treated as an SQL null.
 For records in ROW_FORMAT=COMPACT (new-style records), len must not be
 UNIV_SQL_NULL unless the field already is SQL null. */
void rec_set_nth_field(
    rec_t *rec,           /*!< in: record */
    const ulint *offsets, /*!< in: array returned by rec_get_offsets() */
    ulint n,              /*!< in: index number of the field */
    const void *data,     /*!< in: pointer to the data
                          if not SQL null */
    ulint len)            /*!< in: length of the data or UNIV_SQL_NULL */
{
  byte *data2;
  ulint len2;

  ut_ad(rec);
  ut_ad(rec_offs_validate(rec, NULL, offsets));

  if (len == UNIV_SQL_NULL) {
    if (!rec_offs_nth_sql_null(offsets, n)) {
      ut_a(!rec_offs_comp(offsets));
      rec_set_nth_field_sql_null(rec, n);
    }

    return;
  }

  ut_ad(!rec_offs_nth_default(offsets, n));

  data2 = rec_get_nth_field(rec, offsets, n, &len2);

  if (len2 == UNIV_SQL_NULL) {
    ut_ad(!rec_offs_comp(offsets));
    rec_set_nth_field_null_bit(rec, n, FALSE);
    ut_ad(len == rec_get_nth_field_size(rec, n));
  } else {
    ut_ad(len2 == len);
  }

  ut_memcpy(data2, data, len);
}
