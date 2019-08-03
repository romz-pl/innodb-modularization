#include <innodb/buffer/srv_buf_pool_chunk_unit.h>

/** Requested buffer pool chunk size. Each buffer pool instance consists
of one or more chunks. */
ulonglong srv_buf_pool_chunk_unit;
