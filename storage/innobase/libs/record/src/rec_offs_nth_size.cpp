#include <innodb/record/rec_offs_nth_size.h>

#include <innodb/record/rec_offs_validate.h>
#include <innodb/record/rec_offs_n_fields.h>
#include <innodb/record/rec_offs_base.h>
#include <innodb/record/flag.h>
#include <innodb/assert/assert.h>

/** Gets the physical size of a field.
 @return length of field */
ulint rec_offs_nth_size(
    const ulint *offsets, /*!< in: array returned by rec_get_offsets() */
    ulint n)              /*!< in: nth field */
{
  ut_ad(rec_offs_validate(NULL, NULL, offsets));
  ut_ad(n < rec_offs_n_fields(offsets));
  if (!n) {
    return (rec_offs_base(offsets)[1 + n] & REC_OFFS_MASK);
  }
  return ((rec_offs_base(offsets)[1 + n] - rec_offs_base(offsets)[n]) &
          REC_OFFS_MASK);
}
