#include <innodb/log_recv/recv_sys_close.h>

#include <innodb/log_recv/recv_sys.h>
#include <innodb/allocator/ut_free.h>
#include <innodb/sync_mutex/mutex_free.h>
#include <innodb/log_recv/recv_sys_finish.h>
#include <innodb/sync_event/os_event_destroy.h>
#include <innodb/univ/call_destructor.h>

/** Release recovery system mutexes. */
void recv_sys_close() {
  if (recv_sys == nullptr) {
    return;
  }

  recv_sys_finish();

#ifndef UNIV_HOTBACKUP
  if (recv_sys->flush_start != nullptr) {
    os_event_destroy(recv_sys->flush_start);
  }

  if (recv_sys->flush_end != nullptr) {
    os_event_destroy(recv_sys->flush_end);
  }

  ut_ad(!recv_writer_thread_active);
  mutex_free(&recv_sys->writer_mutex);
#endif /* !UNIV_HOTBACKUP */

  call_destructor(&recv_sys->dblwr);
  call_destructor(&recv_sys->deleted);
  call_destructor(&recv_sys->missing_ids);

  mutex_free(&recv_sys->mutex);

  ut_free(recv_sys);
  recv_sys = nullptr;
}
