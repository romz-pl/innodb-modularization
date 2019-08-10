#include <innodb/log_write/log_flush_low.h>

#include <innodb/io/srv_unix_file_flush_method.h>
#include <innodb/log_types/LOG_SYNC_POINT.h>
#include <innodb/log_types/log_t.h>
#include <innodb/log_write/log_flush_update_stats.h>
#include <innodb/sync_event/os_event_reset.h>
#include <innodb/sync_event/os_event_set.h>
#include <innodb/tablespace/fil_flush_file_redo.h>

void log_flush_low(log_t &log) {
  ut_ad(log_flusher_mutex_own(log));

#ifndef _WIN32
  bool do_flush = srv_unix_file_flush_method != SRV_UNIX_O_DSYNC;
#else
  bool do_flush = true;
#endif

  os_event_reset(log.flusher_event);

  log.last_flush_start_time = Log_clock::now();

  const lsn_t last_flush_lsn = log.flushed_to_disk_lsn.load();

  const lsn_t flush_up_to_lsn = log.write_lsn.load();

  ut_a(flush_up_to_lsn > last_flush_lsn);

  if (do_flush) {
    LOG_SYNC_POINT("log_flush_before_fsync");

    fil_flush_file_redo();
  }

  log.last_flush_end_time = Log_clock::now();

  if (log.last_flush_end_time < log.last_flush_start_time) {
    /* Time was moved backward after we set start_time.
    Let assume that the fsync operation was instant.

    We move start_time backward, because we don't want
    it to remain in the future. */
    log.last_flush_start_time = log.last_flush_end_time;
  }

  LOG_SYNC_POINT("log_flush_before_flushed_to_disk_lsn");

  log.flushed_to_disk_lsn.store(flush_up_to_lsn);

  /* Notify other thread(s). */

  DBUG_PRINT("ib_log", ("Flushed to disk up to " LSN_PF, flush_up_to_lsn));

  const auto first_slot =
      last_flush_lsn / OS_FILE_LOG_BLOCK_SIZE & (log.flush_events_size - 1);

  const auto last_slot = (flush_up_to_lsn - 1) / OS_FILE_LOG_BLOCK_SIZE &
                         (log.flush_events_size - 1);

  if (first_slot == last_slot) {
    LOG_SYNC_POINT("log_flush_before_users_notify");
    os_event_set(log.flush_events[first_slot]);
  } else {
    LOG_SYNC_POINT("log_flush_before_notifier_notify");
    os_event_set(log.flush_notifier_event);
  }

  /* Update stats. */

  log_flush_update_stats(log);
}
