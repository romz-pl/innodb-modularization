#include <innodb/buffer/buf_get_total_list_len.h>

#include <innodb/buffer/buf_pool_t.h>
#include <innodb/buffer/buf_pool_from_array.h>
#include <innodb/lst/lst.h>

/** Requested number of buffer pool instances */
extern ulong srv_buf_pool_instances;

/** Get total buffer pool statistics. */
void buf_get_total_list_len(
    ulint *LRU_len,        /*!< out: length of all LRU lists */
    ulint *free_len,       /*!< out: length of all free lists */
    ulint *flush_list_len) /*!< out: length of all flush lists */
{
  ulint i;

  *LRU_len = 0;
  *free_len = 0;
  *flush_list_len = 0;

  for (i = 0; i < srv_buf_pool_instances; i++) {
    buf_pool_t *buf_pool;

    buf_pool = buf_pool_from_array(i);

    *LRU_len += UT_LIST_GET_LEN(buf_pool->LRU);
    *free_len += UT_LIST_GET_LEN(buf_pool->free);
    *flush_list_len += UT_LIST_GET_LEN(buf_pool->flush_list);
  }
}

