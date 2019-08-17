#include <innodb/page/page_header_set_ptr.h>

#include <innodb/page/page_zip_des_t.h>
#include <innodb/page/page_header_set_field.h>
#include <innodb/page/header.h>

/** Sets the pointer stored in the given header field. */
void page_header_set_ptr(
    page_t *page,             /*!< in: page */
    page_zip_des_t *page_zip, /*!< in/out: compressed page whose
                             uncompressed part will be updated, or NULL */
    ulint field,              /*!< in: PAGE_FREE, ... */
    const byte *ptr)          /*!< in: pointer or NULL*/
{
  ulint offs;

  ut_ad(page);
  ut_ad((field == PAGE_FREE) || (field == PAGE_LAST_INSERT) ||
        (field == PAGE_HEAP_TOP));

  if (ptr == NULL) {
    offs = 0;
  } else {
    offs = ptr - page;
  }

  ut_ad((field != PAGE_HEAP_TOP) || offs);

  page_header_set_field(page, page_zip, field, offs);
}
