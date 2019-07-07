#include <innodb/buffer/buf_pool_from_bpage.h>


#ifndef UNIV_HOTBACKUP

#include <innodb/buffer/buf_pool_ptr.h>

/** Requested number of buffer pool instances */
extern ulong srv_buf_pool_instances;

/** Returns the buffer pool instance given a page instance
 @return buf_pool */
buf_pool_t *buf_pool_from_bpage(
    const buf_page_t *bpage) /*!< in: buffer pool page */
{
  ulint i;
  i = bpage->buf_pool_index;
  ut_ad(i < srv_buf_pool_instances);
  return (&buf_pool_ptr[i]);
}

#endif
