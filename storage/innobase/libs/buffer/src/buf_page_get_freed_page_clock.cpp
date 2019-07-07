#include <innodb/buffer/buf_page_get_freed_page_clock.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/buffer/buf_page_t.h>

/** Reads the freed_page_clock of a buffer block.
 @return freed_page_clock */
ulint buf_page_get_freed_page_clock(const buf_page_t *bpage) /*!< in: block */
{
  /* This is sometimes read without holding any buffer pool mutex. */
  return (bpage->freed_page_clock);
}

#endif
