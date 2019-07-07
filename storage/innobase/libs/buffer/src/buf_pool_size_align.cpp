#include <innodb/buffer/buf_pool_size_align.h>

#ifndef UNIV_HOTBACKUP

#include <algorithm>

/** Requested number of buffer pool instances */
extern ulong srv_buf_pool_instances;

/** Requested buffer pool chunk size. Each buffer pool instance consists
of one or more chunks. */
extern ulonglong srv_buf_pool_chunk_unit;

/** Minimum pool size in bytes */
extern const ulint srv_buf_pool_min_size;

/** Calculate aligned buffer pool size based on srv_buf_pool_chunk_unit,
if needed.
@param[in]	size	size in bytes
@return	aligned size */
ulint buf_pool_size_align(ulint size) {
  const ulint m = srv_buf_pool_instances * srv_buf_pool_chunk_unit;
  size = std::max(size, srv_buf_pool_min_size);

  if (size % m == 0) {
    return (size);
  } else {
    return ((size / m + 1) * m);
  }
}

#endif
