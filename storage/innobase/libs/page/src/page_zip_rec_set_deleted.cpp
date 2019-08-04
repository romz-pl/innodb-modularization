#include <innodb/page/page_zip_rec_set_deleted.h>

#include <innodb/assert/assert.h>
#include <innodb/memory_check/memory_check.h>
#include <innodb/page/flag.h>
#include <innodb/page/page_offset.h>
#include <innodb/page/page_zip_dir_find.h>
#include <innodb/page/page_zip_get_size.h>
#include <innodb/page/page_zip_validate.h>

/** Write the "deleted" flag of a record on a compressed page.  The flag must
 already have been written on the uncompressed page. */
void page_zip_rec_set_deleted(
    page_zip_des_t *page_zip, /*!< in/out: compressed page */
    const byte *rec,          /*!< in: record on the uncompressed page */
    ulint flag)               /*!< in: the deleted flag (nonzero=TRUE) */
{
  byte *slot = page_zip_dir_find(page_zip, page_offset(rec));
  ut_a(slot);
  UNIV_MEM_ASSERT_RW(page_zip->data, page_zip_get_size(page_zip));
  if (flag) {
    *slot |= (PAGE_ZIP_DIR_SLOT_DEL >> 8);
  } else {
    *slot &= ~(PAGE_ZIP_DIR_SLOT_DEL >> 8);
  }
#ifdef UNIV_ZIP_DEBUG
  ut_a(page_zip_validate(page_zip, page_align(rec), NULL));
#endif /* UNIV_ZIP_DEBUG */
}
