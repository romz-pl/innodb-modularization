#include <innodb/record/rec_get_instant_flag_new.h>

#include <innodb/record/rec_get_info_bits.h>
#include <innodb/record/flag.h>

/** The following function tells if a new-style record is instant record or not
@param[in]	rec	new-style record
@return true if it's instant affected */
bool rec_get_instant_flag_new(const rec_t *rec) {
  ulint info = rec_get_info_bits(rec, TRUE);
  return ((info & REC_INFO_INSTANT_FLAG) != 0);
}
