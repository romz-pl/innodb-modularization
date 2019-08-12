#include <innodb/log_recv/recv_writer_thread.h>

#include <innodb/io/srv_read_only_mode.h>
#include <innodb/ioasync/srv_shutdown_state.h>
#include <innodb/log_recv/recv_sys.h>
#include <innodb/log_recv/recv_writer_thread_active.h>
#include <innodb/log_types/log_test.h>
#include <innodb/log_types/recv_recovery_on.h>
#include <innodb/sync_event/os_event_reset.h>
#include <innodb/sync_event/os_event_set.h>
#include <innodb/sync_event/os_event_wait.h>
#include <innodb/sync_mutex/mutex_enter.h>
#include <innodb/sync_mutex/mutex_exit.h>

/** recv_writer thread tasked with flushing dirty pages from the buffer
pools. */
void recv_writer_thread() {
  ut_ad(!srv_read_only_mode);

  /* The code flow is as follows:
  Step 1: In recv_recovery_from_checkpoint_start().
  Step 2: This recv_writer thread is started.
  Step 3: In recv_recovery_from_checkpoint_finish().
  Step 4: Wait for recv_writer thread to complete. This is based
          on the flag recv_writer_thread_active.
  Step 5: Assert that recv_writer thread is not active anymore.

  It is possible that the thread that is started in step 2,
  becomes active only after step 4 and hence the assert in
  step 5 fails.  So mark this thread active only if necessary. */
  mutex_enter(&recv_sys->writer_mutex);

  if (recv_recovery_on) {
    recv_writer_thread_active = true;
  } else {
    mutex_exit(&recv_sys->writer_mutex);
    return;
  }
  mutex_exit(&recv_sys->writer_mutex);

  while (srv_shutdown_state == SRV_SHUTDOWN_NONE) {
    os_thread_sleep(100000);

    mutex_enter(&recv_sys->writer_mutex);

    if (!recv_recovery_on) {
      mutex_exit(&recv_sys->writer_mutex);
      break;
    }

    if (log_test != nullptr) {
      mutex_exit(&recv_sys->writer_mutex);
      continue;
    }

    /* Flush pages from end of LRU if required */
    os_event_reset(recv_sys->flush_end);
    recv_sys->flush_type = BUF_FLUSH_LRU;
    os_event_set(recv_sys->flush_start);
    os_event_wait(recv_sys->flush_end);

    mutex_exit(&recv_sys->writer_mutex);
  }

  recv_writer_thread_active = false;

  my_thread_end();
}
