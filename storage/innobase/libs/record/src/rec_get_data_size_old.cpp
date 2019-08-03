#include <innodb/record/rec_get_data_size_old.h>

#include <innodb/record/rec_get_field_start_offs.h>
#include <innodb/record/rec_get_n_fields_old_raw.h>
#include <innodb/assert/assert.h>

/** The following function returns the data size of an old-style physical
 record, that is the sum of field lengths. SQL null fields
 are counted as length 0 fields. The value returned by the function
 is the distance from record origin to record end in bytes.
 @return size */
ulint rec_get_data_size_old(const rec_t *rec) /*!< in: physical record */
{
  ut_ad(rec);

  return (rec_get_field_start_offs(rec, rec_get_n_fields_old_raw(rec)));
}
