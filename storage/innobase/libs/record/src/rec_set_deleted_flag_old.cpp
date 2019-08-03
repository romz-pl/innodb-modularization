#include <innodb/record/rec_set_deleted_flag_old.h>

#include <innodb/record/rec_get_info_bits.h>
#include <innodb/record/rec_set_info_bits_old.h>
#include <innodb/record/flag.h>

/** The following function is used to set the deleted bit. */
void rec_set_deleted_flag_old(rec_t *rec, /*!< in: old-style physical record */
                              ulint flag) /*!< in: nonzero if delete marked */
{
  ulint val;

  val = rec_get_info_bits(rec, FALSE);

  if (flag) {
    val |= REC_INFO_DELETED_FLAG;
  } else {
    val &= ~REC_INFO_DELETED_FLAG;
  }

  rec_set_info_bits_old(rec, val);
}
