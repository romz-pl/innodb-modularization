#include <innodb/page/page_rec_check.h>

#include <innodb/page/page_align.h>
#include <innodb/page/page_header_get_field.h>
#include <innodb/page/header.h>
#include <innodb/page/page_offset.h>
#include <innodb/assert/assert.h>

/** Used to check the consistency of a record on a page.
 @return true if succeed */
ibool page_rec_check(const rec_t *rec) /*!< in: record */
{
  const page_t *page = page_align(rec);

  ut_a(rec);

  ut_a(page_offset(rec) <= page_header_get_field(page, PAGE_HEAP_TOP));
  ut_a(page_offset(rec) >= PAGE_DATA);

  return (TRUE);
}
