#include <innodb/page/fil_page_get_prev.h>

#include <innodb/page/type.h>
#include <innodb/machine/data.h>

/** Get the predecessor of a file page.
@param[in]	page		File page
@return FIL_PAGE_PREV */
page_no_t fil_page_get_prev(const byte *page) {
  return (mach_read_from_4(page + FIL_PAGE_PREV));
}
