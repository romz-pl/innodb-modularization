#include <innodb/log_write/log_wait_for_flush.h>

#include <innodb/log_types/LOG_SYNC_POINT.h>
#include <innodb/log_types/log_t.h>
#include <innodb/log_write/log_max_spins_when_waiting_in_user_thread.h>
#include <innodb/log_write/srv_log_wait_for_flush_spin_delay.h>
#include <innodb/log_write/srv_log_wait_for_flush_spin_hwm.h>
#include <innodb/log_write/srv_log_wait_for_flush_timeout.h>
#include <innodb/monitor/MONITOR_INC_WAIT_STATS.h>
#include <innodb/sync_event/os_event_set.h>
#include <innodb/sync_event/os_event_wait_for.h>

/** Waits until redo log is flushed up to provided lsn (or greater).
@param[in]	log	redo log
@param[in]	lsn	wait until log.flushed_to_disk_lsn >= lsn
@return		statistics related to waiting inside */
Wait_stats log_wait_for_flush(const log_t &log, lsn_t lsn) {
  if (log.write_lsn.load(std::memory_order_relaxed) < lsn) {
    os_event_set(log.writer_event);
  }
  os_event_set(log.flusher_event);

  uint64_t max_spins = log_max_spins_when_waiting_in_user_thread(
      srv_log_wait_for_flush_spin_delay);

  if (log.flush_avg_time >= srv_log_wait_for_flush_spin_hwm) {
    max_spins = 0;
  }

  auto stop_condition = [&log, lsn](bool wait) {
    LOG_SYNC_POINT("log_wait_for_flush_before_flushed_to_disk_lsn");

    if (log.flushed_to_disk_lsn.load() >= lsn) {
      return (true);
    }

    if (wait) {
      if (log.write_lsn.load(std::memory_order_relaxed) < lsn) {
        os_event_set(log.writer_event);
      }

      os_event_set(log.flusher_event);
    }

    LOG_SYNC_POINT("log_wait_for_flush_before_wait");
    return (false);
  };

  size_t slot =
      (lsn - 1) / OS_FILE_LOG_BLOCK_SIZE & (log.flush_events_size - 1);

  const auto wait_stats =
      os_event_wait_for(log.flush_events[slot], max_spins,
                        srv_log_wait_for_flush_timeout, stop_condition);

  MONITOR_INC_WAIT_STATS(MONITOR_LOG_ON_FLUSH_, wait_stats);

  return (wait_stats);
}
