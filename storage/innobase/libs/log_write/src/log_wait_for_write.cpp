#include <innodb/log_write/log_wait_for_write.h>

#include <innodb/log_types/flags.h>
#include <innodb/log_redo/log_background_write_threads_active_validate.h>
#include <innodb/log_types/log_t.h>
#include <innodb/log_wait/log_max_spins_when_waiting_in_user_thread.h>
#include <innodb/log_wait/srv_log_wait_for_write_spin_delay.h>
#include <innodb/log_wait/srv_log_wait_for_write_timeout.h>
#include <innodb/monitor/MONITOR_INC_WAIT_STATS.h>
#include <innodb/sync_event/os_event_set.h>
#include <innodb/sync_event/os_event_wait_for.h>

/** Waits until redo log is written up to provided lsn (or greater).
We do not care if it's flushed or not.
@param[in]	log	redo log
@param[in]	lsn	wait until log.write_lsn >= lsn
@return		statistics related to waiting inside */
Wait_stats log_wait_for_write(const log_t &log, lsn_t lsn) {
  os_event_set(log.writer_event);

  const uint64_t max_spins = log_max_spins_when_waiting_in_user_thread(
      srv_log_wait_for_write_spin_delay);

  auto stop_condition = [&log, lsn](bool wait) {
    if (log.write_lsn.load() >= lsn) {
      return (true);
    }

    if (wait) {
      os_event_set(log.writer_event);
    }

    ut_d(log_background_write_threads_active_validate(log));
    return (false);
  };

  size_t slot =
      (lsn - 1) / OS_FILE_LOG_BLOCK_SIZE & (log.write_events_size - 1);

  const auto wait_stats =
      os_event_wait_for(log.write_events[slot], max_spins,
                        srv_log_wait_for_write_timeout, stop_condition);

  MONITOR_INC_WAIT_STATS(MONITOR_LOG_ON_WRITE_, wait_stats);

  return (wait_stats);
}
