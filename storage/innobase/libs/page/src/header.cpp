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
