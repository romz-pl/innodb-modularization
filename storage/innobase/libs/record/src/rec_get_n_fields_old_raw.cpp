#include <innodb/record/rec_get_n_fields_old_raw.h>

#include <innodb/record/rec_get_bit_field_2.h>
#include <innodb/record/flag.h>
#include <innodb/assert/assert.h>

/** The following function is used to get the number of fields
 in an old-style record, which is stored in the rec
 @return number of data fields */
MY_ATTRIBUTE((warn_unused_result)) uint16_t
    rec_get_n_fields_old_raw(const rec_t *rec) /*!< in: physical record */
{
  uint16_t ret;

  ut_ad(rec);

  ret = rec_get_bit_field_2(rec, REC_OLD_N_FIELDS, REC_OLD_N_FIELDS_MASK,
                            REC_OLD_N_FIELDS_SHIFT);
  ut_ad(ret <= REC_MAX_N_FIELDS);
  ut_ad(ret > 0);

  return (ret);
}
