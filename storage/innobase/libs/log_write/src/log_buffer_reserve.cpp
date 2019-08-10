#include <innodb/log_write/log_buffer_reserve.h>

#include <innodb/ioasync/srv_shutdown_state.h>
#include <innodb/log_types/LOG_SYNC_POINT.h>
#include <innodb/log_redo/log_background_threads_active_validate.h>
#include <innodb/log_redo/log_buffer_s_lock_enter.h>
#include <innodb/log_sn/log_lsn_validate.h>
#include <innodb/log_sn/log_translate_sn_to_lsn.h>
#include <innodb/log_redo/srv_stats.h>
#include <innodb/log_types/log_t.h>
#include <innodb/log_write/log_wait_for_space_after_reserving.h>

Log_handle log_buffer_reserve(log_t &log, size_t len) {
  Log_handle handle;

  handle.lock_no = log_buffer_s_lock_enter(log);

  /* In 5.7, we incremented log_write_requests for each single
  write to log buffer in commit of mini transaction.

  However, writes which were solved by log_reserve_and_write_fast
  missed to increment the counter. Therefore it wasn't reliable.

  Dimitri and I have decided to change meaning of the counter
  to reflect mtr commit rate. */
  srv_stats.log_write_requests.inc();

  ut_a(srv_shutdown_state <= SRV_SHUTDOWN_FLUSH_PHASE);
  ut_a(len > 0);

  /* Reserve space in sequence of data bytes: */
  const sn_t start_sn = log.sn.fetch_add(len);

  /* Ensure that redo log has been initialized properly. */
  ut_a(start_sn > 0);

#ifdef UNIV_DEBUG
  if (!recv_recovery_is_on()) {
    log_background_threads_active_validate(log);
  } else {
    ut_a(!recv_no_ibuf_operations);
  }
#endif

  /* Headers in redo blocks are not calculated to sn values: */
  const sn_t end_sn = start_sn + len;

  LOG_SYNC_POINT("log_buffer_reserve_before_sn_limit_for_end");

  /* Translate sn to lsn (which includes also headers in redo blocks): */
  handle.start_lsn = log_translate_sn_to_lsn(start_sn);
  handle.end_lsn = log_translate_sn_to_lsn(end_sn);

  if (unlikely(end_sn > log.sn_limit_for_end.load())) {
    log_wait_for_space_after_reserving(log, handle);
  }

  ut_a(log_lsn_validate(handle.start_lsn));
  ut_a(log_lsn_validate(handle.end_lsn));

  return (handle);
}

