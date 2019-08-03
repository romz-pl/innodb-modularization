#include <innodb/record/rec_get_instant_flag_new_temp.h>

#include <innodb/record/rec_get_info_bits_temp.h>
#include <innodb/record/flag.h>

/** The following function tells if a new-style temporary record is instant
record or not
@param[in]	rec	new-style temporary record
@return	true if it's instant affected */
bool rec_get_instant_flag_new_temp(const rec_t *rec) {
  ulint info = rec_get_info_bits_temp(rec);
  return ((info & REC_INFO_INSTANT_FLAG) != 0);
}
