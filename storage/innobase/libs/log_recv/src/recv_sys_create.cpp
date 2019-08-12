#include <innodb/log_recv/recv_sys_create.h>

#include <innodb/log_recv/recv_sys.h>
#include <innodb/allocator/ut_zalloc_nokey.h>
#include <innodb/sync_mutex/mutex_create.h>

/** Creates the recovery system. */
void recv_sys_create() {
  if (recv_sys != nullptr) {
    return;
  }

  recv_sys = static_cast<recv_sys_t *>(ut_zalloc_nokey(sizeof(*recv_sys)));

  mutex_create(LATCH_ID_RECV_SYS, &recv_sys->mutex);
  mutex_create(LATCH_ID_RECV_WRITER, &recv_sys->writer_mutex);

  recv_sys->spaces = nullptr;
}
