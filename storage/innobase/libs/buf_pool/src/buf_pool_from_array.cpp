#include <innodb/buf_pool/buf_pool_from_array.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/buf_pool/buf_pool_t.h>
#include <innodb/buf_pool/buf_pool_ptr.h>
#include <innodb/buf_page/flags.h>

/** Requested number of buffer pool instances */
extern ulong srv_buf_pool_instances;

/** Returns the buffer pool instance given its array index
 @return buffer pool */
buf_pool_t *buf_pool_from_array(ulint index) /*!< in: array index to get
                                             buffer pool instance from */
{
  ut_ad(index < MAX_BUFFER_POOLS);
  ut_ad(index < srv_buf_pool_instances);
  return (&buf_pool_ptr[index]);
}

#endif
