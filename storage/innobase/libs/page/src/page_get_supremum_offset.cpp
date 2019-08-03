#include <innodb/page/page_get_supremum_offset.h>

#include <innodb/page/page_offset.h>
#include <innodb/page/page_is_comp.h>
#include <innodb/page/header.h>
#include <innodb/assert/assert.h>

/** Gets the offset of the last record on the page.
 @return offset of the last record in record list, relative from page */
ulint page_get_supremum_offset(
    const page_t *page) /*!< in: page which must have record(s) */
{
  ut_ad(page);
  ut_ad(!page_offset(page));

  if (page_is_comp(page)) {
    return (PAGE_NEW_SUPREMUM);
  } else {
    return (PAGE_OLD_SUPREMUM);
  }
}
