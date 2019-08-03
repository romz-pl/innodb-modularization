#include <innodb/page/fil_page_set_type.h>

#include <innodb/page/page_type_t.h>
#include <innodb/machine/data.h>

/** Sets the file page type.
@param[in,out]	page		File page
@param[in]	type		Page type */
void fil_page_set_type(byte *page, ulint type) {
  mach_write_to_2(page + FIL_PAGE_TYPE, type);
}
