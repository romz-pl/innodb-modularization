#include <innodb/record/rec_get_info_and_status_bits.h>

#include <innodb/record/rec_get_info_bits.h>
#include <innodb/record/flag.h>
#include <innodb/record/rec_get_status.h>
#include <innodb/assert/assert.h>

/** The following function is used to retrieve the info and status
 bits of a record.  (Only compact records have status bits.)
 @return info bits */
ulint rec_get_info_and_status_bits(
    const rec_t *rec, /*!< in: physical record */
    ulint comp)       /*!< in: nonzero=compact page format */
{
  ulint bits;
#if (REC_NEW_STATUS_MASK >> REC_NEW_STATUS_SHIFT) & \
    (REC_INFO_BITS_MASK >> REC_INFO_BITS_SHIFT)
#error "REC_NEW_STATUS_MASK and REC_INFO_BITS_MASK overlap"
#endif
  if (comp) {
    bits = rec_get_info_bits(rec, TRUE) | rec_get_status(rec);
  } else {
    bits = rec_get_info_bits(rec, FALSE);
    ut_ad(!(bits & ~(REC_INFO_BITS_MASK >> REC_INFO_BITS_SHIFT)));
  }
  return (bits);
}
