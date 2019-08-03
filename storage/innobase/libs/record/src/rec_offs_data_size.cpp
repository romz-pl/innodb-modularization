#include <innodb/record/rec_offs_data_size.h>

#include <innodb/record/rec_offs_validate.h>
#include <innodb/record/rec_offs_base.h>
#include <innodb/record/rec_offs_n_fields.h>
#include <innodb/assert/assert.h>
#include <innodb/record/flag.h>

/** The following function returns the data size of a physical
 record, that is the sum of field lengths. SQL null fields
 are counted as length 0 fields. The value returned by the function
 is the distance from record origin to record end in bytes.
 @return size */
ulint rec_offs_data_size(
    const ulint *offsets) /*!< in: array returned by rec_get_offsets() */
{
  ulint size;

  ut_ad(rec_offs_validate(NULL, NULL, offsets));
  size = rec_offs_base(offsets)[rec_offs_n_fields(offsets)] & REC_OFFS_MASK;
  ut_ad(size < UNIV_PAGE_SIZE);
  return (size);
}
