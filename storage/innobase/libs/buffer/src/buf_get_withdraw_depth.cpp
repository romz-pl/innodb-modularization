#include <innodb/buffer/buf_get_withdraw_depth.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/buffer/buf_pool_t.h>
#include <innodb/sync_mutex/mutex_enter.h>
#include <innodb/sync_mutex/mutex_exit.h>
#include <innodb/atomic/atomic.h>

/** Return how many more pages must be added to the withdraw list to reach the
withdraw target of the currently ongoing buffer pool resize.
@param[in]	buf_pool	buffer pool instance
@return page count to be withdrawn or zero if the target is already achieved or
if the buffer pool is not currently being resized. */
ulint buf_get_withdraw_depth(buf_pool_t *buf_pool) {
  os_rmb;
  if (buf_pool->curr_size >= buf_pool->old_size) return 0;
  mutex_enter(&buf_pool->free_list_mutex);
  ulint withdraw_len = UT_LIST_GET_LEN(buf_pool->withdraw);
  mutex_exit(&buf_pool->free_list_mutex);
  return (buf_pool->withdraw_target > withdraw_len
              ? buf_pool->withdraw_target - withdraw_len
              : 0);
}

#endif
