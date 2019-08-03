#include <innodb/record/rec_set_info_bits_new.h>

#include <innodb/record/rec_info_bits_valid.h>
#include <innodb/record/rec_set_bit_field_1.h>
#include <innodb/record/flag.h>
#include <innodb/assert/assert.h>

/** The following function is used to set the info bits of a record. */
void rec_set_info_bits_new(rec_t *rec, /*!< in/out: new-style physical record */
                           ulint bits) /*!< in: info bits */
{
  ut_ad(rec_info_bits_valid(bits));
  rec_set_bit_field_1(rec, bits, REC_NEW_INFO_BITS, REC_INFO_BITS_MASK,
                      REC_INFO_BITS_SHIFT);
}
