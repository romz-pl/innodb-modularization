#include <innodb/buf_pool/buf_pool_size_align.h>

#ifndef UNIV_HOTBACKUP

#include <algorithm>
#include <innodb/buf_pool/srv_buf_pool_min_size.h>
#include <innodb/buf_pool/srv_buf_pool_chunk_unit.h>

/** Requested number of buffer pool instances */
extern ulong srv_buf_pool_instances;


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
