#include <innodb/page/page_align.h>

#include <innodb/align/ut_align_down.h>

/** Gets the start of a page.
 @return start of the page */
page_t *page_align(const void *ptr) /*!< in: pointer to page frame */
{
  return ((page_t *)ut_align_down(ptr, UNIV_PAGE_SIZE));
}
