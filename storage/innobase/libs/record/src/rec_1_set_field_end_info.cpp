#include <innodb/record/rec_1_set_field_end_info.h>

#include <innodb/record/rec_get_1byte_offs_flag.h>
#include <innodb/record/rec_get_n_fields_old_raw.h>
#include <innodb/assert/assert.h>
#include <innodb/machine/data.h>
#include <innodb/record/flag.h>

/** Sets the field end info for the nth field if the record is stored in the
 1-byte format. */
void rec_1_set_field_end_info(rec_t *rec, /*!< in: record */
                              ulint n,    /*!< in: field index */
                              ulint info) /*!< in: value to set */
{
  ut_ad(rec_get_1byte_offs_flag(rec));
  ut_ad(n < rec_get_n_fields_old_raw(rec));

  mach_write_to_1(rec - (REC_N_OLD_EXTRA_BYTES + n + 1), info);
}
