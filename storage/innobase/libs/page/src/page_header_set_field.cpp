#include <innodb/page/page_header_set_field.h>

#include <innodb/page/page_zip_des_t.h>
#include <innodb/machine/data.h>
#include <innodb/assert/assert.h>
#include <innodb/page/page_zip_write_header.h>
#include <innodb/page/header.h>

/** Sets the given header field. */
void page_header_set_field(
    page_t *page,             /*!< in/out: page */
    page_zip_des_t *page_zip, /*!< in/out: compressed page whose
                             uncompressed part will be updated, or NULL */
    ulint field,              /*!< in: PAGE_N_DIR_SLOTS, ... */
    ulint val)                /*!< in: value */
{
  ut_ad(page);
  ut_ad(field <= PAGE_N_RECS);
  ut_ad(field == PAGE_N_HEAP || val < UNIV_PAGE_SIZE);
  ut_ad(field != PAGE_N_HEAP || (val & 0x7fff) < UNIV_PAGE_SIZE);

  mach_write_to_2(page + PAGE_HEADER + field, val);
  if (page_zip) {
    page_zip_write_header(page_zip, page + PAGE_HEADER + field, 2, NULL);
  }
}
