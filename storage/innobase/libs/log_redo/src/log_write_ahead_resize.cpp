#include <innodb/log_redo/log_write_ahead_resize.h>

#include <innodb/align/ut_uint64_align_down.h>
#include <innodb/log_redo/log_allocate_write_ahead_buffer.h>
#include <innodb/log_redo/log_deallocate_write_ahead_buffer.h>
#include <innodb/log_types/log_writer_mutex_enter.h>
#include <innodb/log_types/log_writer_mutex_exit.h>
#include <innodb/log_types/flags.h>
#include <innodb/log_types/log_t.h>
#include <innodb/log_redo/srv_log_write_ahead_size.h>

void log_write_ahead_resize(log_t &log, size_t new_size) {
  ut_a(new_size >= INNODB_LOG_WRITE_AHEAD_SIZE_MIN);
  ut_a(new_size <= INNODB_LOG_WRITE_AHEAD_SIZE_MAX);

  log_writer_mutex_enter(log);

  log_deallocate_write_ahead_buffer(log);
  srv_log_write_ahead_size = static_cast<ulong>(new_size);

  log.write_ahead_end_offset =
      ut_uint64_align_down(log.write_ahead_end_offset, new_size);

  log_allocate_write_ahead_buffer(log);

  log_writer_mutex_exit(log);
}


