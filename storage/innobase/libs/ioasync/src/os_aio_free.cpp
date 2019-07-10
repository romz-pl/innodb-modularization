#include <innodb/ioasync/os_aio_free.h>

#include <innodb/allocator/ut_free.h>
#include <innodb/io/Blocks.h>
#include <innodb/io/block_cache.h>
#include <innodb/ioasync/AIO.h>
#include <innodb/ioasync/os_aio_n_segments.h>
#include <innodb/ioasync/os_aio_segment_wait_events.h>
#include <innodb/sync_event/os_event_destroy.h>

/** Frees the asynchronous io system. */
void os_aio_free() {
  AIO::shutdown();

  for (ulint i = 0; i < os_aio_n_segments; i++) {
    os_event_destroy(os_aio_segment_wait_events[i]);
  }

  ut_free(os_aio_segment_wait_events);
  os_aio_segment_wait_events = 0;
  os_aio_n_segments = 0;

  for (Blocks::iterator it = block_cache->begin(); it != block_cache->end();
       ++it) {
    ut_a(it->m_in_use == 0);
    ut_free(it->m_ptr);
  }

  UT_DELETE(block_cache);

  block_cache = NULL;
}
