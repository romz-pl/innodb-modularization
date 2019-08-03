#include <innodb/buffer/buf_pool_set_sizes.h>

#include <innodb/buffer/srv_buf_pool_instances.h>
#include <innodb/buffer/buf_pool_from_array.h>
#include <innodb/buffer/srv_buf_pool_curr_size.h>
#include <innodb/buffer/srv_buf_pool_size.h>
#include <innodb/buffer/srv_buf_pool_old_size.h>
#include <innodb/buffer/srv_buf_pool_base_size.h>
#include <innodb/atomic/atomic.h>
#include <innodb/buffer/buf_pool_t.h>


/** Set buffer pool size variables
 Note: It's safe without mutex protection because of startup only. */
void buf_pool_set_sizes(void) {
  ulint i;
  ulint curr_size = 0;

  for (i = 0; i < srv_buf_pool_instances; i++) {
    buf_pool_t *buf_pool;

    buf_pool = buf_pool_from_array(i);
    curr_size += buf_pool->curr_pool_size;
  }
  if (srv_buf_pool_curr_size == 0) {
    srv_buf_pool_curr_size = curr_size;
  } else {
    srv_buf_pool_curr_size = srv_buf_pool_size;
  }
  srv_buf_pool_old_size = srv_buf_pool_size;
  srv_buf_pool_base_size = srv_buf_pool_size;
  os_wmb;
}
