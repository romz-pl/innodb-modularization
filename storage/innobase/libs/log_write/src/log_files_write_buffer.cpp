#include <innodb/log_write/log_files_write_buffer.h>

#include <innodb/log_types/LOG_SYNC_POINT.h>
#include <innodb/log_redo/srv_stats.h>
#include <innodb/log_types/log_t.h>
#include <innodb/log_write/compute_how_much_to_write.h>
#include <innodb/log_write/compute_how_much_to_write.h>
#include <innodb/log_write/compute_real_offset.h>
#include <innodb/log_write/copy_to_write_ahead_buffer.h>
#include <innodb/log_write/current_write_ahead_enough.h>
#include <innodb/log_write/prepare_for_write_ahead.h>
#include <innodb/log_write/prepare_full_blocks.h>
#include <innodb/log_write/start_next_file.h>
#include <innodb/log_write/update_current_write_ahead.h>
#include <innodb/log_write/validate_buffer.h>
#include <innodb/log_write/validate_start_lsn.h>
#include <innodb/log_write/write_blocks.h>
#include <innodb/monitor/MONITOR_INC_VALUE.h>
#include <innodb/monitor/MONITOR_SET.h>

void notify_about_advanced_write_lsn(log_t &log, lsn_t old_write_lsn, lsn_t new_write_lsn);

void log_files_write_buffer(log_t &log, byte *buffer, size_t buffer_size,
                                   lsn_t start_lsn) {
  ut_ad(log_writer_mutex_own(log));

  using namespace Log_files_write_impl;

  validate_buffer(log, buffer, buffer_size);

  validate_start_lsn(log, start_lsn, buffer_size);

  checkpoint_no_t checkpoint_no = log.next_checkpoint_no.load();

  const auto real_offset = compute_real_offset(log, start_lsn);

  bool write_from_log_buffer;

  auto write_size = compute_how_much_to_write(log, real_offset, buffer_size,
                                              write_from_log_buffer);

  if (write_size == 0) {
    start_next_file(log, start_lsn);
    return;
  }

  prepare_full_blocks(log, buffer, write_size, start_lsn, checkpoint_no);

  byte *write_buf;
  uint64_t written_ahead = 0;
  lsn_t lsn_advance = write_size;

  if (write_from_log_buffer) {
    /* We have at least one completed log block to write.
    We write completed blocks from the log buffer. Note,
    that possibly we do not write all completed blocks,
    because of write-ahead strategy (described earlier). */

    write_buf = buffer;

    LOG_SYNC_POINT("log_writer_before_write_from_log_buffer");

  } else {
    write_buf = log.write_ahead_buf;

    /* We write all the data directly from the write-ahead buffer,
    where we first need to copy the data. */
    copy_to_write_ahead_buffer(log, buffer, write_size, start_lsn,
                               checkpoint_no);

    if (!current_write_ahead_enough(log, real_offset, 1)) {
      written_ahead = prepare_for_write_ahead(log, real_offset, write_size);
    }
  }

  srv_stats.os_log_pending_writes.inc();

  /* Now, we know, that we are going to write completed
  blocks only (originally or copied and completed). */
  write_blocks(log, write_buf, write_size, real_offset);

  LOG_SYNC_POINT("log_writer_before_lsn_update");

  const lsn_t old_write_lsn = log.write_lsn.load();

  const lsn_t new_write_lsn = start_lsn + lsn_advance;
  ut_a(new_write_lsn > log.write_lsn.load());

  log.write_lsn.store(new_write_lsn);

  notify_about_advanced_write_lsn(log, old_write_lsn, new_write_lsn);

  srv_stats.os_log_pending_writes.dec();
  srv_stats.log_writes.inc();

  /* Write ahead is included in write_size. */
  ut_a(write_size >= written_ahead);
  srv_stats.os_log_written.add(write_size - written_ahead);
  MONITOR_INC_VALUE(MONITOR_LOG_PADDED, written_ahead);

  int64_t free_space = log.lsn_capacity_for_writer - log.extra_margin;

  /* The free space may be negative (up to -log.extra_margin), in which
  case we are in the emergency mode, eating the extra margin and asking
  to increase concurrency_margin. */
  free_space -= new_write_lsn - log.last_checkpoint_lsn.load();

  MONITOR_SET(MONITOR_LOG_FREE_SPACE, free_space);

  log.n_log_ios++;

  update_current_write_ahead(log, real_offset, write_size);
}

