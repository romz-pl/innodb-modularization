#include <innodb/page/page_zip_write_header.h>

#include <innodb/page/page_zip_des_t.h>
#include <innodb/page/page_zip_simple_validate.h>
#include <innodb/page/page_zip_write_header_log.h>
#include <innodb/assert/assert.h>
#include <innodb/page/page_zip_get_size.h>
#include <innodb/page/page_offset.h>
#include <innodb/memory_check/memory_check.h>

/** Write data to the uncompressed header portion of a page.  The data must
already have been written to the uncompressed page.
@param[in,out]	page_zip	compressed page
@param[in]	str		address on the uncompressed page
@param[in]	length		length of the data
@param[in]	mtr		mini-transaction, or NULL */
void page_zip_write_header(page_zip_des_t *page_zip, const byte *str,
                           ulint length, mtr_t *mtr) {
  ulint pos;

  ut_ad(page_zip_simple_validate(page_zip));
  UNIV_MEM_ASSERT_RW(page_zip->data, page_zip_get_size(page_zip));

  pos = page_offset(str);

  ut_ad(pos < PAGE_DATA);

  memcpy(page_zip->data + pos, str, length);

  /* The following would fail in page_cur_insert_rec_zip(). */
  /* ut_ad(page_zip_validate(page_zip, str - pos)); */

  if (mtr) {
#ifndef UNIV_HOTBACKUP
    page_zip_write_header_log(str, length, mtr);
#endif /* !UNIV_HOTBACKUP */
  }
}
