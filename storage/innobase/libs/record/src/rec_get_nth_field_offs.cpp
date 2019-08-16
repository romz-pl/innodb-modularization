#include <innodb/record/rec_get_nth_field_offs.h>

#include <innodb/record/rec_offs_n_fields.h>
#include <innodb/record/rec_offs_base.h>
#include <innodb/record/flag.h>
#include <innodb/assert/assert.h>
#include <innodb/record/rec_get_instant_offset.h>

/** The following function is used to get an offset to the nth
 data field in a record.
 @return offset from the origin of rec */
ulint rec_get_nth_field_offs(
    const ulint *offsets, /*!< in: array returned by rec_get_offsets() */
    ulint n,              /*!< in: index of the field */
    ulint *len)           /*!< out: length of the field; UNIV_SQL_NULL
                          if SQL null; UNIV_SQL_ADD_COL_DEFAULT if it's default
                          value and no value inlined */
{
  ulint offs;
  ulint length;
  ut_ad(n < rec_offs_n_fields(offsets));
  ut_ad(len);

  if (n == 0) {
    offs = 0;
  } else {
    offs = rec_offs_base(offsets)[n] & REC_OFFS_MASK;
  }

  length = rec_offs_base(offsets)[1 + n];

  if (length & REC_OFFS_SQL_NULL) {
    length = UNIV_SQL_NULL;
  } else if (length & REC_OFFS_DEFAULT) {
    length = UNIV_SQL_ADD_COL_DEFAULT;
  } else {
    length &= REC_OFFS_MASK;
    length -= offs;
  }

  *len = length;
  return (offs);
}
