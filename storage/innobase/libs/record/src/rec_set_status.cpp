#include <innodb/record/rec_set_status.h>

#include <innodb/record/rec_set_bit_field_1.h>
#include <innodb/record/flag.h>

/** The following function is used to set the status bits of a new-style record.
 */
void rec_set_status(rec_t *rec, /*!< in/out: physical record */
                    ulint bits) /*!< in: info bits */
{
  rec_set_bit_field_1(rec, bits, REC_NEW_STATUS, REC_NEW_STATUS_MASK,
                      REC_NEW_STATUS_SHIFT);
}
