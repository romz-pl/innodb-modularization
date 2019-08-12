#include <innodb/log_recv/recv_sys_free.h>

#include <innodb/log_recv/recv_sys.h>
#include <innodb/log_recv/recv_sys_finish.h>
#include <innodb/sync_mutex/mutex_enter.h>
#include <innodb/sync_mutex/mutex_exit.h>
#include <innodb/io/srv_read_only_mode.h>
#include <innodb/log_types/recv_recovery_on.h>
#include <innodb/log_recv/recv_writer_thread_active.h>
#include <innodb/sync_event/os_event_reset.h>
#include <innodb/sync_event/os_event_set.h>

/** Event to synchronise with the flushing. */
extern os_event_t buf_flush_event;

/** Frees the recovery system. */
void recv_sys_free() {
  mutex_enter(&recv_sys->mutex);

  recv_sys_finish();

  /* wake page cleaner up to progress */
  if (!srv_read_only_mode) {
    ut_ad(!recv_recovery_on);
    ut_ad(!recv_writer_thread_active);
    if (buf_flush_event != nullptr) {
      os_event_reset(buf_flush_event);
    }
    os_event_set(recv_sys->flush_start);
  }

  /* Free encryption data structures. */
  if (recv_sys->keys != nullptr) {
    for (auto &key : *recv_sys->keys) {
      if (key.ptr != nullptr) {
        ut_free(key.ptr);
        key.ptr = nullptr;
      }

      if (key.iv != nullptr) {
        ut_free(key.iv);
        key.iv = nullptr;
      }
    }

    recv_sys->keys->swap(*recv_sys->keys);

    UT_DELETE(recv_sys->keys);
    recv_sys->keys = nullptr;
  }

  mutex_exit(&recv_sys->mutex);
}
