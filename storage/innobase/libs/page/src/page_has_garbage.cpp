#include <innodb/page/page_has_garbage.h>

#include <innodb/page/header.h>

/** Determine whether the page contains garbage.
 @return true if the page contains garbage (PAGE_GARBAGE is not 0) */
bool page_has_garbage(const page_t *page) /*!< in: page */
{
  return (!!*(const uint16 *)(page + (PAGE_HEADER + PAGE_GARBAGE)));
}
