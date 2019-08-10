#include <innodb/log_redo/log_print.h>

#include <innodb/log_redo/log_buffer_dirty_pages_added_up_to_lsn.h>
#include <innodb/log_redo/log_buffer_ready_for_write_lsn.h>
#include <innodb/log_types/log_get_lsn.h>
#include <innodb/log_sn/LSN_PF.h>
#include <innodb/log_types/log_t.h>

#include <ctime>

void log_print(const log_t &log, FILE *file) {
  lsn_t last_checkpoint_lsn;
  lsn_t dirty_pages_added_up_to_lsn;
  lsn_t ready_for_write_lsn;
  lsn_t write_lsn;
  lsn_t flush_lsn;
  lsn_t oldest_lsn;
  lsn_t max_assigned_lsn;
  lsn_t current_lsn;

  last_checkpoint_lsn = log.last_checkpoint_lsn;
  dirty_pages_added_up_to_lsn = log_buffer_dirty_pages_added_up_to_lsn(log);
  ready_for_write_lsn = log_buffer_ready_for_write_lsn(log);
  write_lsn = log.write_lsn;
  flush_lsn = log.flushed_to_disk_lsn;
  oldest_lsn = log.available_for_checkpoint_lsn;
  max_assigned_lsn = log_get_lsn(log);
  current_lsn = log_get_lsn(log);

  fprintf(file,
          "Log sequence number          " LSN_PF
          "\n"
          "Log buffer assigned up to    " LSN_PF
          "\n"
          "Log buffer completed up to   " LSN_PF
          "\n"
          "Log written up to            " LSN_PF
          "\n"
          "Log flushed up to            " LSN_PF
          "\n"
          "Added dirty pages up to      " LSN_PF
          "\n"
          "Pages flushed up to          " LSN_PF
          "\n"
          "Last checkpoint at           " LSN_PF "\n",
          current_lsn, max_assigned_lsn, ready_for_write_lsn, write_lsn,
          flush_lsn, dirty_pages_added_up_to_lsn, oldest_lsn,
          last_checkpoint_lsn);

  time_t current_time = time(nullptr);

  double time_elapsed = difftime(current_time, log.last_printout_time);

  if (time_elapsed <= 0) {
    time_elapsed = 1;
  }

  fprintf(
      file, ULINTPF " log i/o's done, %.2f log i/o's/second\n",
      ulint(log.n_log_ios),
      static_cast<double>(log.n_log_ios - log.n_log_ios_old) / time_elapsed);

  log.n_log_ios_old = log.n_log_ios;
  log.last_printout_time = current_time;
}
