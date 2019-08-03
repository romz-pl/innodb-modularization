#include <innodb/page/page_is_leaf.h>

#include <innodb/page/header.h>

/** Determine whether the page is a B-tree leaf.
 @return true if the page is a B-tree leaf (PAGE_LEVEL = 0) */
bool page_is_leaf(const page_t *page) /*!< in: page */
{
  return (!*(const uint16 *)(page + (PAGE_HEADER + PAGE_LEVEL)));
}
