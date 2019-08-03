#include <innodb/record/rec_get_info_bits.h>

#include <innodb/record/rec_get_bit_field_1.h>
#include <innodb/record/rec_info_bits_valid.h>
#include <innodb/record/flag.h>
#include <innodb/assert/assert.h>

/** The following function is used to retrieve the info bits of a record.
@param[in]	rec	physical record
@param[in]	comp	nonzero=compact page format
@return info bits */
ulint rec_get_info_bits(const rec_t *rec, ulint comp) {
  const ulint val =
      rec_get_bit_field_1(rec, comp ? REC_NEW_INFO_BITS : REC_OLD_INFO_BITS,
                          REC_INFO_BITS_MASK, REC_INFO_BITS_SHIFT);
  ut_ad(rec_info_bits_valid(val));
  return (val);
}
