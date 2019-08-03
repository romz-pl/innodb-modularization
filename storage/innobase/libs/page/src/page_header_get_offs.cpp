#include <innodb/page/page_header_get_offs.h>

#include <innodb/page/page_header_get_field.h>
#include <innodb/assert/assert.h>
#include <innodb/page/header.h>

/** Returns the offset stored in the given header field.
 @return offset from the start of the page, or 0 */
ulint page_header_get_offs(const page_t *page, /*!< in: page */
                           ulint field)        /*!< in: PAGE_FREE, ... */
{
  ulint offs;

  ut_ad(page);
  ut_ad((field == PAGE_FREE) || (field == PAGE_LAST_INSERT) ||
        (field == PAGE_HEAP_TOP));

  offs = page_header_get_field(page, field);

  ut_ad((field != PAGE_HEAP_TOP) || offs);

  return (offs);
}
