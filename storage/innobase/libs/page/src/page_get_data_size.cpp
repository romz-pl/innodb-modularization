#include <innodb/page/page_get_data_size.h>

#include <innodb/page/page_header_get_field.h>
#include <innodb/page/page_is_comp.h>
#include <innodb/page/header.h>
#include <innodb/assert/assert.h>

/** Returns the sum of the sizes of the records in the record list, excluding
 the infimum and supremum records.
 @return data in bytes */
ulint page_get_data_size(const page_t *page) /*!< in: index page */
{
  ulint ret;

  ret = (ulint)(
      page_header_get_field(page, PAGE_HEAP_TOP) -
      (page_is_comp(page) ? PAGE_NEW_SUPREMUM_END : PAGE_OLD_SUPREMUM_END) -
      page_header_get_field(page, PAGE_GARBAGE));

  ut_ad(ret < UNIV_PAGE_SIZE);

  return (ret);
}
