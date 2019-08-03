#include <innodb/buffer/innobase_should_madvise_buf_pool.h>

#include <innodb/buffer/srv_buffer_pool_in_core_file.h>

#include "sql/mysqld.h"

/** Checks sys_vars and determines if allocator should mark
large memory segments with MADV_DONTDUMP
@return true iff @@global.core_file AND
NOT @@global.innodb_buffer_pool_in_core_file */
bool innobase_should_madvise_buf_pool() {
  return (test_flags & TEST_CORE_ON_SIGNAL) && !srv_buffer_pool_in_core_file;
}
