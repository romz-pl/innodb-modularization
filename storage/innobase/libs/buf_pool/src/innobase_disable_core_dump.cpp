#include <innodb/buf_pool/innobase_disable_core_dump.h>

#include <innodb/buf_pool/srv_buffer_pool_in_core_file.h>

#include "sql/mysqld.h"

/** Make sure that core file will not be generated, as generating a core file
might violate our promise to not dump buffer pool data, and/or might dump not
the expected memory pages due to failure in using madvise */
void innobase_disable_core_dump() {
  /* TODO: There is a race condition here, as test_flags is not an atomic<>
  and there might be multiple threads calling this function
  in parallel (once for each buffer pool thread).
  One approach would be to use a loop with os_compare_and_swap_ulint
  unfortunately test_flags is defined as uint, not ulint, and we don't
  have nice portable function for dealing with uint in InnoDB.
  Moreover that would only prevent problems with mangled bits, but not
  help at all with that some other thread might be reading test_flags
  and making decisions based on observed value while we are changing it.
  The good news is that all these threads try to do the same thing: clear the
  same bit. So this happens to work.
  */

  test_flags &= ~TEST_CORE_ON_SIGNAL;
}
