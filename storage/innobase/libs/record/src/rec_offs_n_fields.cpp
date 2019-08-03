#include <innodb/record/rec_offs_n_fields.h>

#include <innodb/record/flag.h>
#include <innodb/record/rec_offs_get_n_alloc.h>
#include <innodb/assert/assert.h>

/** The following function returns the number of fields in a record.
 @return number of fields */
MY_ATTRIBUTE((warn_unused_result)) ulint rec_offs_n_fields(
    const ulint *offsets) /*!< in: array returned by rec_get_offsets() */
{
  ulint n_fields;
  ut_ad(offsets);
  n_fields = offsets[1];
  ut_ad(n_fields > 0);
  ut_ad(n_fields <= REC_MAX_N_FIELDS);
  ut_ad(n_fields + REC_OFFS_HEADER_SIZE <= rec_offs_get_n_alloc(offsets));
  return (n_fields);
}
