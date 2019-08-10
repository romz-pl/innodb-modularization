#include <innodb/log_write/log_wait_for_space_in_log_buf.h>

#include <innodb/log_sn/log_translate_lsn_to_sn.h>
#include <innodb/log_sn/log_translate_lsn_to_sn.h>
#include <innodb/log_sn/log_translate_sn_to_lsn.h>
#include <innodb/log_redo/srv_stats.h>
#include <innodb/log_types/log_t.h>
#include <innodb/monitor/MONITOR_INC_WAIT_STATS.h>
#include <innodb/wait/Wait_stats.h>
#include <innodb/log_types/LOG_SYNC_POINT.h>
#include <innodb/log_write/log_write_up_to.h>

void log_wait_for_space_in_log_buf(log_t &log, sn_t end_sn) {
  lsn_t lsn;
  Wait_stats wait_stats;

  const sn_t write_sn = log_translate_lsn_to_sn(log.write_lsn.load());

  LOG_SYNC_POINT("log_wait_for_space_in_buf_middle");

  const sn_t buf_size_sn = log.buf_size_sn.load();

  if (end_sn + OS_FILE_LOG_BLOCK_SIZE <= write_sn + buf_size_sn) {
    return;
  }

  /* We preserve this counter for backward compatibility with 5.7. */
  srv_stats.log_waits.inc();

  lsn = log_translate_sn_to_lsn(end_sn + OS_FILE_LOG_BLOCK_SIZE - buf_size_sn);

  wait_stats = log_write_up_to(log, lsn, false);

  MONITOR_INC_WAIT_STATS(MONITOR_LOG_ON_BUFFER_SPACE_, wait_stats);

  ut_a(end_sn + OS_FILE_LOG_BLOCK_SIZE <=
       log_translate_lsn_to_sn(log.write_lsn.load()) + buf_size_sn);
}
