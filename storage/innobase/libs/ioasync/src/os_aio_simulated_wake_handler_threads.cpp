#include <innodb/ioasync/os_aio_simulated_wake_handler_threads.h>

#include <innodb/ioasync/AIO.h>
#include <innodb/io/srv_use_native_aio.h>
#include <innodb/ioasync/os_aio_recommend_sleep_for_read_threads.h>
#include <innodb/ioasync/os_aio_n_segments.h>

/** Wakes up simulated aio i/o-handler threads if they have something to do. */
void os_aio_simulated_wake_handler_threads() {
  if (srv_use_native_aio) {
    /* We do not use simulated aio: do nothing */

    return;
  }

  os_aio_recommend_sleep_for_read_threads = false;

  for (ulint i = 0; i < os_aio_n_segments; i++) {
    AIO::wake_simulated_handler_thread(i);
  }
}
