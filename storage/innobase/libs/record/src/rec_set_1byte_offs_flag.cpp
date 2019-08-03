#include <innodb/record/rec_set_1byte_offs_flag.h>

#include <innodb/record/rec_set_bit_field_1.h>
#include <innodb/record/flag.h>
#include <innodb/assert/assert.h>

/** The following function is used to set the 1-byte offsets flag. */
void rec_set_1byte_offs_flag(rec_t *rec, /*!< in: physical record */
                             ibool flag) /*!< in: TRUE if 1byte form */
{
#if TRUE != 1
#error "TRUE != 1"
#endif
  ut_ad(flag <= TRUE);

  rec_set_bit_field_1(rec, flag, REC_OLD_SHORT, REC_OLD_SHORT_MASK,
                      REC_OLD_SHORT_SHIFT);
}
