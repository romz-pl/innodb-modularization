#include <innodb/buffer/buf_get_total_list_size_in_bytes.h>

#include <innodb/buffer/buf_pools_list_size_t.h>
#include <innodb/buffer/buf_pool_from_array.h>
#include <innodb/buffer/buf_pool_t.h>
#include <innodb/lst/lst.h>

#include <string.h>

/** Requested number of buffer pool instances */
extern ulong srv_buf_pool_instances;

/** Get total list size in bytes from all buffer pools. */
void buf_get_total_list_size_in_bytes(
    buf_pools_list_size_t *buf_pools_list_size) /*!< out: list sizes
                                                in all buffer pools */
{
  ut_ad(buf_pools_list_size);
  memset(buf_pools_list_size, 0, sizeof(*buf_pools_list_size));

  for (ulint i = 0; i < srv_buf_pool_instances; i++) {
    buf_pool_t *buf_pool;

    buf_pool = buf_pool_from_array(i);
    /* We don't need mutex protection since this is
    for statistics purpose */
    buf_pools_list_size->LRU_bytes += buf_pool->stat.LRU_bytes;
    buf_pools_list_size->unzip_LRU_bytes +=
        UT_LIST_GET_LEN(buf_pool->unzip_LRU) * UNIV_PAGE_SIZE;
    buf_pools_list_size->flush_list_bytes += buf_pool->stat.flush_list_bytes;
  }
}

