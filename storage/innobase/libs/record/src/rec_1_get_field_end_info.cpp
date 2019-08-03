#include <innodb/record/rec_1_get_field_end_info.h>

#include <innodb/record/rec_get_1byte_offs_flag.h>
#include <innodb/record/rec_get_n_fields_old_raw.h>
#include <innodb/record/flag.h>
#include <innodb/machine/data.h>
#include <innodb/assert/assert.h>

/** Returns the offset of nth field end if the record is stored in the 1-byte
 offsets form. If the field is SQL null, the flag is ORed in the returned
 value.
 @return offset of the start of the field, SQL null flag ORed */
MY_ATTRIBUTE((warn_unused_result)) ulint
    rec_1_get_field_end_info(const rec_t *rec, /*!< in: record */
                             ulint n)          /*!< in: field index */
{
  ut_ad(rec_get_1byte_offs_flag(rec));
  ut_ad(n < rec_get_n_fields_old_raw(rec));

  return (mach_read_from_1(rec - (REC_N_OLD_EXTRA_BYTES + n + 1)));
}
