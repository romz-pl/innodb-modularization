#include <innodb/page/page_offset.h>

#include <innodb/align/ut_align_offset.h>

/** Gets the offset within a page.
 @return offset from the start of the page */
ulint page_offset(const void *ptr) /*!< in: pointer to page frame */
{
  return (ut_align_offset(ptr, UNIV_PAGE_SIZE));
}
