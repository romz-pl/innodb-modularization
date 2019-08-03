#include <innodb/record/rec_offs_any_extern.h>

#include <innodb/record/rec_offs_validate.h>
#include <innodb/record/rec_offs_base.h>
#include <innodb/record/flag.h>
#include <innodb/assert/assert.h>

/** Determine if the offsets are for a record containing
 externally stored columns.
 @return nonzero if externally stored */
ulint rec_offs_any_extern(
    const ulint *offsets) /*!< in: array returned by rec_get_offsets() */
{
  ut_ad(rec_offs_validate(NULL, NULL, offsets));
  return (*rec_offs_base(offsets) & REC_OFFS_EXTERNAL);
}
