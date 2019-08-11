#include <innodb/buf_pool/buf_pool_get_curr_size.h>

#include <innodb/buf_pool/srv_buf_pool_curr_size.h>

/** Gets the current size of buffer buf_pool in bytes.
 @return size in bytes */
ulint buf_pool_get_curr_size(void) {
    return (srv_buf_pool_curr_size);
}
