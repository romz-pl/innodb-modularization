#include <innodb/ioasync/os_aio_wake_all_threads_at_shutdown.h>

#include <innodb/io/srv_use_native_aio.h>
#include <innodb/ioasync/os_aio_n_segments.h>
#include <innodb/ioasync/os_aio_segment_wait_events.h>
#include <innodb/sync_event/os_event_set.h>

/** Wakes up all async i/o threads so that they know to exit themselves in
shutdown. */
void os_aio_wake_all_threads_at_shutdown() {
#ifdef WIN_ASYNC_IO

  AIO::wake_at_shutdown();

#elif defined(LINUX_NATIVE_AIO)

  /* When using native AIO interface the io helper threads
  wait on io_getevents with a timeout value of 500ms. At
  each wake up these threads check the server status.
  No need to do anything to wake them up. */

  if (srv_use_native_aio) {
    return;
  }

#endif /* !WIN_ASYNC_AIO */

  /* Fall through to simulated AIO handler wakeup if we are
  not using native AIO. */

  /* This loop wakes up all simulated ai/o threads */

  for (ulint i = 0; i < os_aio_n_segments; ++i) {
    os_event_set(os_aio_segment_wait_events[i]);
  }
}
