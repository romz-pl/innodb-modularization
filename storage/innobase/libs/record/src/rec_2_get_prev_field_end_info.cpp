#include <innodb/record/rec_2_get_prev_field_end_info.h>

#include <innodb/record/rec_get_1byte_offs_flag.h>
#include <innodb/record/rec_get_n_fields_old_raw.h>
#include <innodb/record/flag.h>
#include <innodb/assert/assert.h>
#include <innodb/machine/data.h>

/** Returns the offset of n - 1th field end if the record is stored in the
 2-byte offsets form. If the field is SQL null, the flag is ORed in the returned
 value.
 @return offset of the start of the PREVIOUS field, SQL null flag ORed */
ulint rec_2_get_prev_field_end_info(const rec_t *rec, /*!< in: record */
                                    ulint n)          /*!< in: field index */
{
  ut_ad(!rec_get_1byte_offs_flag(rec));
  ut_ad(n <= rec_get_n_fields_old_raw(rec));

  return (mach_read_from_2(rec - (REC_N_OLD_EXTRA_BYTES + 2 * n)));
}
