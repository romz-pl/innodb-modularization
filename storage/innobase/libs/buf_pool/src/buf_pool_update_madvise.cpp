#include <innodb/buf_pool/buf_pool_update_madvise.h>

#include <innodb/buf_pool/srv_buf_pool_instances.h>
#include <innodb/buf_pool/buf_pool_should_madvise.h>
#include <innodb/buf_pool/innobase_disable_core_dump.h>
#include <innodb/buf_pool/buf_pool_from_array.h>
#include <innodb/buf_pool/innobase_should_madvise_buf_pool.h>
#include <innodb/buf_pool/buf_pool_t.h>
#include <innodb/sync_mutex/mutex_exit.h>
#include <innodb/sync_mutex/mutex_enter.h>

/** Checks if innobase_should_madvise_buf_pool() value has changed since we've
last check and if so, then updates buf_pool_should_madvise and calls madvise
for all chunks in all srv_buf_pool_instances.
@see buf_pool_should_madvise comment for a longer explanation. */
void buf_pool_update_madvise() {
  /* We need to make sure that buf_pool_should_madvise value change does not
  occur in parallel with allocation or deallocation of chunks in some buf_pool
  as this could lead to inconsistency - we would call madvise for some but not
  all chunks, perhaps with a wrong MADV_DO(NT)_DUMP flag.
  Moreover, we are about to iterate over chunks, which requires the bounds of
  for loop to be fixed.
  To solve both problems we first latch all buf_pool_t::chunks_mutex-es, and
  only then update the buf_pool_should_madvise, and perform iteration over
  buf_pool-s and their chunks.*/
  for (ulint i = 0; i < srv_buf_pool_instances; i++) {
    mutex_enter(&buf_pool_from_array(i)->chunks_mutex);
  }

  auto should_madvise = innobase_should_madvise_buf_pool();
  /* This `if` is here not for performance, but for correctness: on platforms
  which do not support madvise MADV_DONT_DUMP we prefer to not call madvice to
  avoid warnings and disabling @@global.core_file in cases where the user did
  not really intend to change anything */
  if (should_madvise != buf_pool_should_madvise) {
    buf_pool_should_madvise = should_madvise;
    for (ulint i = 0; i < srv_buf_pool_instances; i++) {
      buf_pool_t *buf_pool = buf_pool_from_array(i);
      bool success = buf_pool_should_madvise ? buf_pool->madvise_dont_dump()
                                             : buf_pool->madvise_dump();
      if (!success) {
        innobase_disable_core_dump();
        break;
      }
    }
  }
  for (ulint i = 0; i < srv_buf_pool_instances; i++) {
    mutex_exit(&buf_pool_from_array(i)->chunks_mutex);
  }
}
