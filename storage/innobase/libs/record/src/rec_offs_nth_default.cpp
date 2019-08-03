#include <innodb/record/rec_offs_nth_default.h>

#include <innodb/record/rec_offs_validate.h>
#include <innodb/record/rec_offs_n_fields.h>
#include <innodb/record/rec_offs_base.h>
#include <innodb/record/flag.h>
#include <innodb/assert/assert.h>

/** Returns nonzero if the default bit is set in nth field of rec.
@return nonzero if default bit is set */
ulint rec_offs_nth_default(const ulint *offsets, ulint n) {
  ut_ad(rec_offs_validate(NULL, NULL, offsets));
  ut_ad(n < rec_offs_n_fields(offsets));
  return (rec_offs_base(offsets)[1 + n] & REC_OFFS_DEFAULT);
}
