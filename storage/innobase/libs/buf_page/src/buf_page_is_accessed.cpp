#include <innodb/buf_page/buf_page_is_accessed.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/buf_page/buf_page_t.h>
#include <innodb/buf_page/buf_page_in_file.h>

/** Determine the time of first access of a block in the buffer pool.
 @return ut_time_ms() at the time of first access, 0 if not accessed */
unsigned buf_page_is_accessed(const buf_page_t *bpage) /*!< in: control block */
{
  ut_ad(buf_page_in_file(bpage));

  return (bpage->access_time);
}


#endif
