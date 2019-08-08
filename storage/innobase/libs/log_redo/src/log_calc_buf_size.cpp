#include <innodb/log_redo/log_calc_buf_size.h>

#include <innodb/log_types/log_t.h>
#include <innodb/log_redo/flags.h>
#include <innodb/log_redo/srv_log_buffer_size.h>
#include <innodb/log_redo/log_translate_lsn_to_sn.h>
#include <innodb/log_redo/log_update_limits.h>

void log_calc_buf_size(log_t &log) {
  ut_a(srv_log_buffer_size >= INNODB_LOG_BUFFER_SIZE_MIN);
  ut_a(srv_log_buffer_size <= INNODB_LOG_BUFFER_SIZE_MAX);

  log.buf_size = srv_log_buffer_size;

  /* The following update has to be the last operation during resize
  procedure of log buffer. That's because since this moment, possibly
  new concurrent writes for higher sn will start (which were waiting
  for free space in the log buffer). */

  log.buf_size_sn = log_translate_lsn_to_sn(log.buf_size);

  log_update_limits(log);
}
