#include <innodb/record/rec_set_n_fields_old.h>

#include <innodb/record/rec_set_bit_field_2.h>
#include <innodb/record/flag.h>
#include <innodb/assert/assert.h>

/** The following function is used to set the number of fields
 in an old-style record. */
void rec_set_n_fields_old(rec_t *rec,     /*!< in: physical record */
                          ulint n_fields) /*!< in: the number of fields */
{
  ut_ad(rec);
  ut_ad(n_fields <= REC_MAX_N_FIELDS);
  ut_ad(n_fields > 0);

  rec_set_bit_field_2(rec, n_fields, REC_OLD_N_FIELDS, REC_OLD_N_FIELDS_MASK,
                      REC_OLD_N_FIELDS_SHIFT);
}
