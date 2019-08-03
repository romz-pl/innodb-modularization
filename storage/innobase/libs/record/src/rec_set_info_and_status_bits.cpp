#include <innodb/record/rec_set_info_and_status_bits.h>

#include <innodb/record/flag.h>
#include <innodb/record/rec_set_status.h>
#include <innodb/record/rec_set_info_bits_new.h>

/** The following function is used to set the info and status
 bits of a record.  (Only compact records have status bits.) */
void rec_set_info_and_status_bits(rec_t *rec, /*!< in/out: physical record */
                                  ulint bits) /*!< in: info bits */
{
#if (REC_NEW_STATUS_MASK >> REC_NEW_STATUS_SHIFT) & \
    (REC_INFO_BITS_MASK >> REC_INFO_BITS_SHIFT)
#error "REC_NEW_STATUS_MASK and REC_INFO_BITS_MASK overlap"
#endif
  rec_set_status(rec, bits & REC_NEW_STATUS_MASK);
  rec_set_info_bits_new(rec, bits & ~REC_NEW_STATUS_MASK);
}
