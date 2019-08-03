#include <innodb/record/rec_get_status.h>

#include <innodb/record/rec_get_bit_field_1.h>
#include <innodb/record/flag.h>
#include <innodb/assert/assert.h>

/** The following function retrieves the status bits of a new-style record.
 @return status bits */
MY_ATTRIBUTE((warn_unused_result)) ulint
    rec_get_status(const rec_t *rec) /*!< in: physical record */
{
  ulint ret;

  ut_ad(rec);

  ret = rec_get_bit_field_1(rec, REC_NEW_STATUS, REC_NEW_STATUS_MASK,
                            REC_NEW_STATUS_SHIFT);
  ut_ad((ret & ~REC_NEW_STATUS_MASK) == 0);

  return (ret);
}
