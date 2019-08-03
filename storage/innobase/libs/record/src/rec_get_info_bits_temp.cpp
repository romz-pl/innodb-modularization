#include <innodb/record/rec_get_info_bits_temp.h>

#include <innodb/record/rec_get_bit_field_1.h>
#include <innodb/record/rec_info_bits_valid.h>
#include <innodb/record/flag.h>
#include <innodb/assert/assert.h>

/** The following function is used to retrieve the info bits of a temporary
record.
@param[in]	rec	physical record
@return	info bits */
ulint rec_get_info_bits_temp(const rec_t *rec) {
  const ulint val = rec_get_bit_field_1(
      rec, REC_TMP_INFO_BITS, REC_INFO_BITS_MASK, REC_INFO_BITS_SHIFT);
  ut_ad(rec_info_bits_valid(val));
  return (val);
}
