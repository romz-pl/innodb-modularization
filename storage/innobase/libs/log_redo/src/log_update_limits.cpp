#include <innodb/log_redo/log_update_limits.h>

#include <innodb/log_redo/LOG_SYNC_POINT.h>
#include <innodb/log_redo/flags.h>
#include <innodb/log_redo/log_free_check_margin.h>
#include <innodb/log_redo/log_translate_lsn_to_sn.h>
#include <innodb/log_types/log_t.h>

void log_update_limits(log_t &log) {
  const sn_t sn_file_limit = log_translate_lsn_to_sn(
      log.last_checkpoint_lsn.load() + log.lsn_capacity_for_free_check);

  LOG_SYNC_POINT("log_update_limits_middle_1");

  const lsn_t write_lsn = log.write_lsn.load();

  LOG_SYNC_POINT("log_update_limits_middle_2");

  const sn_t limit_for_end = log_translate_lsn_to_sn(write_lsn) +
                             log.buf_size_sn.load() -
                             2 * OS_FILE_LOG_BLOCK_SIZE;

  log.sn_limit_for_end.store(limit_for_end);

  LOG_SYNC_POINT("log_update_limits_middle_3");

  sn_t limit_for_start = std::min(sn_file_limit, limit_for_end);

  const sn_t margins = log_free_check_margin(log);

  if (margins >= limit_for_start) {
    limit_for_start = 0;
  } else {
    limit_for_start -= margins;
  }
  log.sn_limit_for_start.store(limit_for_start);
}

