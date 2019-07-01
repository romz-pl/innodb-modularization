#include <innodb/page/header.h>

#include <innodb/machine/data.h>
#include <innodb/assert/assert.h>

/** Reads the given header field. */
ulint page_header_get_field(const page_t *page, /*!< in: page */
                            ulint field)        /*!< in: PAGE_LEVEL, ... */
{
  ut_ad(page);
  ut_ad(field <= PAGE_INDEX_ID);

  return (mach_read_from_2(page + PAGE_HEADER + field));
}

/** Gets the number of records in the heap.
 @return number of user records */
ulint page_dir_get_n_heap(const page_t *page) /*!< in: index page */
{
  return (page_header_get_field(page, PAGE_N_HEAP) & 0x7fff);
}
