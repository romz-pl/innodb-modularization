#include <innodb/page/page_is_empty.h>

#include <innodb/page/header.h>

/** Determine whether the page is empty.
 @return true if the page is empty (PAGE_N_RECS = 0) */
bool page_is_empty(const page_t *page) /*!< in: page */
{
  return (!*(const uint16 *)(page + (PAGE_HEADER + PAGE_N_RECS)));
}
