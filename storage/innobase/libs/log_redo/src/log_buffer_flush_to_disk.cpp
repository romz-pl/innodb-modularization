#include <innodb/log_redo/log_buffer_flush_to_disk.h>

#include <innodb/log_redo/log_get_lsn.h>
#include <innodb/log_types/recv_recovery_is_on.h>


extern bool srv_read_only_mode;


#ifndef UNIV_HOTBACKUP

#include <innodb/log_redo/log_sys.h>

void log_buffer_flush_to_disk(bool sync) {
  log_buffer_flush_to_disk(*log_sys, sync);
}

#endif

#include <innodb/wait/Wait_stats.h>
Wait_stats log_write_up_to(log_t &log, lsn_t lsn, bool flush_to_disk);

void log_buffer_flush_to_disk(log_t &log, bool sync) {
  ut_a(!srv_read_only_mode);
  ut_a(!recv_recovery_is_on());

  const lsn_t lsn = log_get_lsn(log);

  log_write_up_to(log, lsn, sync);
}
