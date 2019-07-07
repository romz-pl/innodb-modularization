#include <innodb/buffer/buf_pool_index.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/buffer/buf_pool_ptr.h>
#include <innodb/buffer/macros.h>

/** Requested number of buffer pool instances */
extern ulong srv_buf_pool_instances;

/** Calculates the index of a buffer pool to the buf_pool[] array.
 @return the position of the buffer pool in buf_pool[] */
ulint buf_pool_index(const buf_pool_t *buf_pool) /*!< in: buffer pool */
{
  ulint i = buf_pool - buf_pool_ptr;
  ut_ad(i < MAX_BUFFER_POOLS);
  ut_ad(i < srv_buf_pool_instances);
  return (i);
}

#endif
