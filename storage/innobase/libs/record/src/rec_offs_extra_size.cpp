#include <innodb/record/rec_offs_extra_size.h>

#include <innodb/record/rec_offs_validate.h>
#include <innodb/record/rec_offs_base.h>
#include <innodb/record/flag.h>
#include <innodb/assert/assert.h>

/** Returns the total size of record minus data size of record. The value
 returned by the function is the distance from record start to record origin
 in bytes.
 @return size */
ulint rec_offs_extra_size(
    const ulint *offsets) /*!< in: array returned by rec_get_offsets() */
{
  ulint size;
  ut_ad(rec_offs_validate(NULL, NULL, offsets));
  size = *rec_offs_base(offsets) & ~(REC_OFFS_COMPACT | REC_OFFS_EXTERNAL);
  ut_ad(size < UNIV_PAGE_SIZE);
  return (size);
}
