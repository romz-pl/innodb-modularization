#include <innodb/buf_pool/srv_buf_pool_old_size.h>

/** Previously requested size. Accesses protected by memory barriers. */
ulint srv_buf_pool_old_size = 0;

