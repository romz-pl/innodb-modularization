#include <innodb/record/rec_offs_comp.h>

#include <innodb/record/rec_offs_validate.h>
#include <innodb/record/rec_offs_base.h>
#include <innodb/record/flag.h>
#include <innodb/assert/assert.h>

/** Determine if the offsets are for a record in the new
 compact format.
 @return nonzero if compact format */
ulint rec_offs_comp(
    const ulint *offsets) /*!< in: array returned by rec_get_offsets() */
{
  ut_ad(rec_offs_validate(NULL, NULL, offsets));
  return (*rec_offs_base(offsets) & REC_OFFS_COMPACT);
}
