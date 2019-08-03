#include <innodb/record/rec_offs_nth_extern.h>

#include <innodb/record/rec_offs_validate.h>
#include <innodb/record/rec_offs_n_fields.h>
#include <innodb/record/rec_offs_base.h>
#include <innodb/record/flag.h>
#include <innodb/assert/assert.h>

/** Returns nonzero if the extern bit is set in nth field of rec.
 @return nonzero if externally stored */
ulint rec_offs_nth_extern(
    const ulint *offsets, /*!< in: array returned by rec_get_offsets() */
    ulint n)              /*!< in: nth field */
{
  ut_ad(rec_offs_validate(NULL, NULL, offsets));
  ut_ad(n < rec_offs_n_fields(offsets));
  return (rec_offs_base(offsets)[1 + n] & REC_OFFS_EXTERNAL);
}
