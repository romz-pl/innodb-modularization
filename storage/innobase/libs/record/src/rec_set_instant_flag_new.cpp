#include <innodb/record/rec_set_instant_flag_new.h>

#include <innodb/record/rec_get_info_bits.h>
#include <innodb/record/rec_set_info_bits_new.h>
#include <innodb/record/flag.h>

/** The following function is used to set the instant bit.
@param[in,out]	rec	new-style physical record
@param[in]	flag	set the bit to this flag */
void rec_set_instant_flag_new(rec_t *rec, bool flag) {
  ulint val;

  val = rec_get_info_bits(rec, TRUE);

  if (flag) {
    val |= REC_INFO_INSTANT_FLAG;
  } else {
    val &= ~REC_INFO_INSTANT_FLAG;
  }

  rec_set_info_bits_new(rec, val);
}
