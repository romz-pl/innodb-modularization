#include <innodb/page/page_get_page_no.h>

#include <innodb/page/page_align.h>
#include <innodb/page/header.h>
#include <innodb/machine/data.h>
#include <innodb/assert/assert.h>

/** Gets the page number.
 @return page number */
page_no_t page_get_page_no(const page_t *page) /*!< in: page */
{
  ut_ad(page == page_align((page_t *)page));
  return (mach_read_from_4(page + FIL_PAGE_OFFSET));
}
