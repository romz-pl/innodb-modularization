#include <innodb/log_sys/log_sys_close.h>

#include <innodb/log_files/log_deallocate_file_header_buffers.h>
#include <innodb/log_redo/log_deallocate_write_events.h>
#include <innodb/log_flush/log_deallocate_flush_events.h>
#include <innodb/log_redo/log_deallocate_recent_closed.h>
#include <innodb/log_redo/log_deallocate_recent_written.h>
#include <innodb/log_chkp/log_deallocate_checkpoint_buffer.h>
#include <innodb/log_redo/log_deallocate_write_ahead_buffer.h>
#include <innodb/log_buffer/log_deallocate_buffer.h>
#include <innodb/log_types/log_sys.h>
#include <innodb/sync_mutex/mutex_free.h>
#include <innodb/sync_event/os_event_destroy.h>
#include <innodb/log_sys/log_sys_object.h>

void log_sys_close() {
  ut_a(log_sys != nullptr);

  log_t &log = *log_sys;

  log_deallocate_file_header_buffers(log);
  log_deallocate_write_events(log);
  log_deallocate_flush_events(log);
  log_deallocate_recent_closed(log);
  log_deallocate_recent_written(log);
  log_deallocate_checkpoint_buffer(log);
  log_deallocate_write_ahead_buffer(log);
  log_deallocate_buffer(log);

  log.sn_lock.free();

  mutex_free(&log.write_notifier_mutex);
  mutex_free(&log.flush_notifier_mutex);
  mutex_free(&log.flusher_mutex);
  mutex_free(&log.writer_mutex);
  mutex_free(&log.closer_mutex);
  mutex_free(&log.checkpointer_mutex);

  os_event_destroy(log.write_notifier_event);
  os_event_destroy(log.flush_notifier_event);
  os_event_destroy(log.closer_event);
  os_event_destroy(log.checkpointer_event);
  os_event_destroy(log.writer_event);
  os_event_destroy(log.flusher_event);

  log_sys_object->destroy();

  ut_free(log_sys_object);
  log_sys_object = nullptr;

  log_sys = nullptr;
}
