#include <innodb/record/rec_set_deleted_flag_new.h>

#include <innodb/page/page_zip_des_t.h>
#include <innodb/record/rec_get_info_bits.h>
#include <innodb/record/rec_set_info_bits_new.h>
#include <innodb/page/page_zip_rec_set_deleted.h>
#include <innodb/record/flag.h>

/** The following function is used to set the deleted bit. */
void rec_set_deleted_flag_new(
    rec_t *rec,               /*!< in/out: new-style physical record */
    page_zip_des_t *page_zip, /*!< in/out: compressed page, or NULL */
    ulint flag)               /*!< in: nonzero if delete marked */
{
  ulint val;

  val = rec_get_info_bits(rec, TRUE);

  if (flag) {
    val |= REC_INFO_DELETED_FLAG;
  } else {
    val &= ~REC_INFO_DELETED_FLAG;
  }

  rec_set_info_bits_new(rec, val);

  if (page_zip) {
    page_zip_rec_set_deleted(page_zip, rec, flag);
  }
}
