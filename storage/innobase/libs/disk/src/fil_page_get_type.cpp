#include <innodb/disk/fil_page_get_type.h>

#include <innodb/disk/page_type_t.h>
#include <innodb/machine/data.h>

/** Get the file page type.
@param[in]	page		File page
@return page type */
page_type_t fil_page_get_type(const byte *page) {
  return (static_cast<page_type_t>(mach_read_from_2(page + FIL_PAGE_TYPE)));
}
