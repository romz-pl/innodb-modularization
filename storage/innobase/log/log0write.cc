/*****************************************************************************

Copyright (c) 1995, 2019, Oracle and/or its affiliates. All Rights Reserved.
Copyright (c) 2009, Google Inc.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2.0,
as published by the Free Software Foundation.

This program is also distributed with certain software (including
but not limited to OpenSSL) that is licensed under separate terms,
as designated in a particular file or component or in included license
documentation.  The authors of MySQL hereby grant you an additional
permission to link the program and your derivative works with the
separately licensed software that they have included with MySQL.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License, version 2.0, for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

Portions of this file contain modifications contributed and copyrighted by
Google, Inc. Those modifications are gratefully acknowledged and are described
briefly in the InnoDB documentation. The contributions by Google are
incorporated with their permission, and subject to the conditions contained in
the file COPYING.Google.

*****************************************************************************/

/**************************************************/ /**
 @file log/log0write.cc

 Redo log writing and flushing, including functions for:
         1. Waiting for the log written / flushed up to provided lsn.
         2. Redo log background threads (except the log checkpointer).

 @author Paweł Olchawa

 *******************************************************/

#ifndef UNIV_HOTBACKUP

#include <cstring>

#include <innodb/log_write/log_file_header_fill_encryption.h>
#include <innodb/log_write/log_wait_for_write.h>
#include <innodb/log_write/srv_log_wait_for_write_spin_delay.h>
#include <innodb/log_write/log_max_spins_when_waiting_in_user_thread.h>
#include <innodb/log_redo/log_closer_mutex_exit.h>
#include <innodb/log_redo/log_closer_mutex_enter.h>
#include <innodb/log_redo/log_flush_notifier_mutex_exit.h>
#include <innodb/log_redo/log_flush_notifier_mutex_enter.h>
#include <innodb/log_redo/log_write_notifier_mutex_exit.h>
#include <innodb/log_redo/log_write_notifier_mutex_enter.h>
#include <innodb/log_redo/log_flusher_mutex_exit.h>
#include <innodb/log_redo/log_flusher_mutex_enter.h>
#include <innodb/log_redo/log_writer_mutex_enter.h>
#include <innodb/log_redo/log_writer_mutex_exit.h>
#include <innodb/allocator/ut_malloc_nokey.h>
#include <innodb/logger/info.h>
#include <innodb/align/ut_uint64_align_up.h>
#include <innodb/align/ut_uint64_align_down.h>
#include <innodb/sync_event/os_event_wait_low.h>
#include <innodb/sync_event/os_event_reset.h>
#include <innodb/sync_event/os_event_wait_for.h>
#include <innodb/io/IORequestLogRead.h>
#include <innodb/io/IORequestLogWrite.h>
#include <innodb/log_redo/log_files_header_flush.h>
#include <innodb/log_write/validate_buffer.h>
#include <innodb/log_write/validate_start_lsn.h>
#include <innodb/log_write/compute_real_offset.h>
#include <innodb/log_write/current_file_has_space.h>
#include <innodb/log_write/start_next_file.h>
#include <innodb/log_write/write_ahead_enough.h>
#include <innodb/log_write/current_write_ahead_enough.h>
#include <innodb/log_write/compute_next_write_ahead_end.h>
#include <innodb/log_write/compute_how_much_to_write.h>
#include <innodb/log_write/prepare_full_blocks.h>
#include <innodb/log_write/write_blocks.h>
#include <innodb/log_write/compute_write_event_slot.h>
#include <innodb/log_write/copy_to_write_ahead_buffer.h>
#include <innodb/log_write/prepare_for_write_ahead.h>
#include <innodb/log_write/update_current_write_ahead.h>
#include <innodb/log_write/log_wait_for_flush.h>
#include <innodb/log_write/Log_thread_waiting.h>
#include <innodb/log_write/Log_write_to_file_requests_monitor.h>


#include "ha_prototypes.h"

#include <debug_sync.h>

#include "arch0arch.h"
#include "buf0buf.h"
#include "buf0flu.h"
#include "dict0boot.h"
#include "dict0stats_bg.h"
#endif /* !UNIV_HOTBACKUP */
#include "fil0fil.h"
#include "log0log.h"
#ifndef UNIV_HOTBACKUP
#include "log0recv.h"

#include "mysqld.h" /* server_uuid */
#include "srv0mon.h"
#include "srv0srv.h"
#include "srv0start.h"

#include "trx0roll.h"
#include "trx0sys.h"
#include "trx0trx.h"


/** Writes fragment of log buffer to log files. The first write to the first
log block in a new log file, flushes header of the file. It stops after doing
single fil_io operation. The reason is that it might make sense to advance
lsn up to which we have ready data in log buffer for write, after time
consuming operation, such as fil_io. The log.write_lsn is advanced.
@param[in]  log            redo log
@param[in]  buffer         the beginning of first log block to write
@param[in]  buffer_size    number of bytes to write since 'buffer'
@param[in]  start_lsn	lsn  corresponding to first block start */
static void log_files_write_buffer(log_t &log, byte *buffer, size_t buffer_size,
                                   lsn_t start_lsn);

/* Waits until there is free space in log files for log_writer to proceed.
@param[in]  log             redo log
@param[in]  last_write_lsn  previous log.write_lsn
@param[in]  next_write_lsn  next log.write_lsn
@return lsn up to which possible write is limited */
static lsn_t log_writer_wait_on_checkpoint(log_t &log, lsn_t last_write_lsn,
                                           lsn_t next_write_lsn);

/* Waits until the archiver has archived enough for log_writer to proceed
or until the archiver becomes aborted.
@param[in]  log             redo log
@param[in]  last_write_lsn  previous log.write_lsn
@param[in]  next_write_lsn  next log.write_lsn */
static void log_writer_wait_on_archiver(log_t &log, lsn_t last_write_lsn,
                                        lsn_t next_write_lsn);

/** Writes fragment of the log buffer up to provided lsn (not further).
Stops after the first call to fil_io() (possibly at smaller lsn).
Main side-effect: log.write_lsn is advanced.
@param[in]  log             redo log
@param[in]  next_write_lsn  write up to this lsn value */
static void log_writer_write_buffer(log_t &log, lsn_t next_write_lsn);

/** Executes a synchronous flush of the log files (doing fsyncs).
Advances log.flushed_to_disk_lsn and notifies log flush_notifier thread.
Note: if only a single log block was flushed to disk, user threads
waiting for lsns within the block are notified directly from here,
and log flush_notifier thread is not notified! (optimization)
@param[in,out]	log   redo log */
static void log_flush_low(log_t &log);



/**************************************************/ /**

 @name Log writer thread

 *******************************************************/

/* @{ */

#else /* !UNIV_HOTBACKUP */
#define log_writer_mutex_own(log) true
#endif /* !UNIV_HOTBACKUP */






#ifndef UNIV_HOTBACKUP



namespace Log_files_write_impl {


static inline void notify_about_advanced_write_lsn(log_t &log,
                                                   lsn_t old_write_lsn,
                                                   lsn_t new_write_lsn) {
  if (srv_flush_log_at_trx_commit == 1) {
    os_event_set(log.flusher_event);
  }

  const auto first_slot = compute_write_event_slot(log, old_write_lsn);

  const auto last_slot = compute_write_event_slot(log, new_write_lsn);

  if (first_slot == last_slot) {
    LOG_SYNC_POINT("log_write_before_users_notify");
    os_event_set(log.write_events[first_slot]);
  } else {
    LOG_SYNC_POINT("log_write_before_notifier_notify");
    os_event_set(log.write_notifier_event);
  }

  if (arch_log_sys && arch_log_sys->is_active()) {
    os_event_set(log_archiver_thread_event);
  }
}





}  // namespace Log_files_write_impl





static void log_files_write_buffer(log_t &log, byte *buffer, size_t buffer_size,
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

static lsn_t log_writer_wait_on_checkpoint(log_t &log, lsn_t last_write_lsn,
                                           lsn_t next_write_lsn) {
  const int32_t SLEEP_BETWEEN_RETRIES_IN_US = 100; /* 100us */

  const int32_t TIME_UNTIL_ERROR_IN_US = 5000000; /* 5s */

  ut_ad(log_writer_mutex_own(log));

  int32_t count = 1;
  lsn_t checkpoint_limited_lsn = LSN_MAX;

  while (true) {
    lsn_t checkpoint_lsn = log.last_checkpoint_lsn.load();

    checkpoint_lsn =
        ut_uint64_align_down(checkpoint_lsn, OS_FILE_LOG_BLOCK_SIZE);

    checkpoint_limited_lsn = checkpoint_lsn + log.lsn_capacity_for_writer;

    ut_a(last_write_lsn <= checkpoint_limited_lsn);
    ut_a(next_write_lsn > checkpoint_lsn);

    if (next_write_lsn + log.extra_margin <= checkpoint_limited_lsn) {
      log.concurrency_margin_ok = true;
      break;
    }

    if (log.concurrency_margin_ok) {
      log.concurrency_margin_ok = false;
      log_increase_concurrency_margin(log);
    }

    if (last_write_lsn + OS_FILE_LOG_BLOCK_SIZE <= checkpoint_limited_lsn) {
      /* Write what we have - adjust the speed to speed of checkpoints
      going forward (to speed of page-cleaners). */
      break;
    }

    (void)log_advance_ready_for_write_lsn(log);

    const int32_t ATTEMPTS_UNTIL_ERROR =
        TIME_UNTIL_ERROR_IN_US / SLEEP_BETWEEN_RETRIES_IN_US;

    if (count % ATTEMPTS_UNTIL_ERROR == 0) {
      /* We could not reclaim even single redo block for 5sec */

      ib::error(ER_IB_MSG_1234) << "Out of space in the redo log."
                                   " Checkpoint LSN: "
                                << checkpoint_lsn << ".";
    }

    log_writer_mutex_exit(log);

    /* We don't want to ask for sync checkpoint, because it
    is possible, that the oldest dirty page is latched and
    user thread, which keeps the latch, is waiting for space
    in log buffer (for log_writer writing to disk). In such
    case it would be deadlock (we can't flush the latched
    page and advance the checkpoint). We only ask for the
    checkpoint, and wait for some time. */
    log_request_checkpoint(log, false);

    count++;
    os_thread_sleep(SLEEP_BETWEEN_RETRIES_IN_US);

    MONITOR_INC(MONITOR_LOG_WRITER_ON_FREE_SPACE_WAITS);

    log_writer_mutex_enter(log);
  }

  return checkpoint_limited_lsn;
}

static void log_writer_wait_on_archiver(log_t &log, lsn_t last_write_lsn,
                                        lsn_t next_write_lsn) {
  const int32_t SLEEP_BETWEEN_RETRIES_IN_US = 100; /* 100us */

  const int32_t TIME_BETWEEN_WARNINGS_IN_US = 100000; /* 100ms */

  const int32_t TIME_UNTIL_ERROR_IN_US = 1000000; /* 1s */

  ut_ad(log_writer_mutex_own(log));

  int32_t count = 0;

  while (arch_log_sys != nullptr && arch_log_sys->is_active()) {
    lsn_t archiver_lsn = arch_log_sys->get_archived_lsn();

    archiver_lsn = ut_uint64_align_down(archiver_lsn, OS_FILE_LOG_BLOCK_SIZE);

    lsn_t archiver_limited_lsn = archiver_lsn + log.lsn_capacity_for_writer;

    ut_a(next_write_lsn > archiver_lsn);

    if (next_write_lsn <= archiver_limited_lsn) {
      /* Between archive_lsn and next_write_lsn there is less
      bytes than capacity of all log files. Writing log up to
      next_write_lsn will not overwrite data at archiver_lsn.
      There is no need to wait for the archiver. */
      break;
    }

    (void)log_advance_ready_for_write_lsn(log);

    const int32_t ATTEMPTS_UNTIL_ERROR =
        TIME_UNTIL_ERROR_IN_US / SLEEP_BETWEEN_RETRIES_IN_US;

    if (count >= ATTEMPTS_UNTIL_ERROR) {
      log_writer_mutex_exit(log);

      arch_log_sys->force_abort();

      const lsn_t lag = next_write_lsn - archiver_limited_lsn;

      ib::error(ER_IB_MSG_1236)
          << "Log writer waited too long for redo-archiver"
             " to advance (1 second). There are unarchived: "
          << lag << " bytes. Archiver LSN: " << archiver_lsn
          << ". Aborted the redo-archiver.";

      log_writer_mutex_enter(log);
      break;
    }

    os_event_set(log_archiver_thread_event);

    log_writer_mutex_exit(log);

    const int32_t ATTEMPTS_BETWEEN_WARNINGS =
        TIME_BETWEEN_WARNINGS_IN_US / SLEEP_BETWEEN_RETRIES_IN_US;

    if (count % ATTEMPTS_BETWEEN_WARNINGS == 0) {
      const lsn_t lag = next_write_lsn - archiver_limited_lsn;

      ib::warn(ER_IB_MSG_1237)
          << "Log writer is waiting for redo-archiver"
             " to catch up unarchived: "
          << lag << " bytes. Archiver LSN: " << archiver_lsn << ".";
    }

    count++;
    os_thread_sleep(SLEEP_BETWEEN_RETRIES_IN_US);

    MONITOR_INC(MONITOR_LOG_WRITER_ON_ARCHIVER_WAITS);

    log_writer_mutex_enter(log);
  }
}

static void log_writer_write_buffer(log_t &log, lsn_t next_write_lsn) {
  ut_ad(log_writer_mutex_own(log));

  LOG_SYNC_POINT("log_writer_write_begin");

  const lsn_t last_write_lsn = log.write_lsn.load();

  ut_a(log_lsn_validate(last_write_lsn) ||
       last_write_lsn % OS_FILE_LOG_BLOCK_SIZE == 0);

  ut_a(log_lsn_validate(next_write_lsn) ||
       next_write_lsn % OS_FILE_LOG_BLOCK_SIZE == 0);

  ut_a(next_write_lsn - last_write_lsn <= log.buf_size);
  ut_a(next_write_lsn > last_write_lsn);

  size_t start_offset = last_write_lsn % log.buf_size;
  size_t end_offset = next_write_lsn % log.buf_size;

  if (start_offset >= end_offset) {
    ut_a(next_write_lsn - last_write_lsn >= log.buf_size - start_offset);

    end_offset = log.buf_size;
    next_write_lsn = last_write_lsn + (end_offset - start_offset);
  }
  ut_a(start_offset < end_offset);

  ut_a(end_offset % OS_FILE_LOG_BLOCK_SIZE == 0 ||
       end_offset % OS_FILE_LOG_BLOCK_SIZE >= LOG_BLOCK_HDR_SIZE);

  /* Wait until there is free space in log files.*/

  const lsn_t checkpoint_limited_lsn =
      log_writer_wait_on_checkpoint(log, last_write_lsn, next_write_lsn);

  ut_ad(log_writer_mutex_own(log));
  ut_a(checkpoint_limited_lsn > last_write_lsn);

  LOG_SYNC_POINT("log_writer_after_checkpoint_check");

  if (arch_log_sys != nullptr) {
    log_writer_wait_on_archiver(log, last_write_lsn, next_write_lsn);
  }

  ut_ad(log_writer_mutex_own(log));

  LOG_SYNC_POINT("log_writer_after_archiver_check");

  const lsn_t limit_for_next_write_lsn = checkpoint_limited_lsn;

  if (limit_for_next_write_lsn < next_write_lsn) {
    end_offset -= next_write_lsn - limit_for_next_write_lsn;
    next_write_lsn = limit_for_next_write_lsn;

    ut_a(end_offset > start_offset);
    ut_a(end_offset % OS_FILE_LOG_BLOCK_SIZE == 0 ||
         end_offset % OS_FILE_LOG_BLOCK_SIZE >= LOG_BLOCK_HDR_SIZE);

    ut_a(log_lsn_validate(next_write_lsn) ||
         next_write_lsn % OS_FILE_LOG_BLOCK_SIZE == 0);
  }

  DBUG_PRINT("ib_log",
             ("write " LSN_PF " to " LSN_PF, last_write_lsn, next_write_lsn));

  byte *buf_begin =
      log.buf + ut_uint64_align_down(start_offset, OS_FILE_LOG_BLOCK_SIZE);

  byte *buf_end = log.buf + end_offset;

  /* Do the write to the log files */
  log_files_write_buffer(
      log, buf_begin, buf_end - buf_begin,
      ut_uint64_align_down(last_write_lsn, OS_FILE_LOG_BLOCK_SIZE));

  LOG_SYNC_POINT("log_writer_before_limits_update");

  log_update_limits(log);

  LOG_SYNC_POINT("log_writer_write_end");
}

void log_writer(log_t *log_ptr) {
  ut_a(log_ptr != nullptr);
  ut_a(log_ptr->writer_thread_alive.load());

  log_t &log = *log_ptr;
  lsn_t ready_lsn = 0;

  log_writer_mutex_enter(log);

  Log_thread_waiting waiting{log, log.writer_event, srv_log_writer_spin_delay,
                             srv_log_writer_timeout};

  Log_write_to_file_requests_monitor write_to_file_requests_monitor{log};

  for (uint64_t step = 0;; ++step) {
    bool released = false;

    auto stop_condition = [&ready_lsn, &log, &released,
                           &write_to_file_requests_monitor](bool wait) {

      if (released) {
        log_writer_mutex_enter(log);
        released = false;
      }

      /* Advance lsn up to which data is ready in log buffer. */
      (void)log_advance_ready_for_write_lsn(log);

      ready_lsn = log_buffer_ready_for_write_lsn(log);

      /* Wait until any of following conditions holds:
              1) There is some unwritten data in log buffer
              2) We should close threads. */

      if (log.write_lsn.load() < ready_lsn || log.should_stop_threads.load()) {
        return (true);
      }

      if (wait) {
        write_to_file_requests_monitor.update();
        log_writer_mutex_exit(log);
        released = true;
      }

      return (false);
    };

    const auto wait_stats = waiting.wait(stop_condition);

    MONITOR_INC_WAIT_STATS(MONITOR_LOG_WRITER_, wait_stats);

    /* Do the actual work. */
    if (log.write_lsn.load() < ready_lsn) {
      log_writer_write_buffer(log, ready_lsn);

      if (step % 1024 == 0) {
        write_to_file_requests_monitor.update();

        log_writer_mutex_exit(log);

        os_thread_sleep(0);

        log_writer_mutex_enter(log);
      }

    } else {
      if (log.should_stop_threads.load()) {
        /* When log threads are stopped, we must first
        ensure that all writes to log buffer have been
        finished and only then we are allowed to set
        the should_stop_threads to true. */

        if (!log_advance_ready_for_write_lsn(log)) {
          break;
        }

        ready_lsn = log_buffer_ready_for_write_lsn(log);
      }
    }
  }

  log.writer_thread_alive.store(false);

  os_event_set(log.write_notifier_event);
  os_event_set(log.flusher_event);

  log_writer_mutex_exit(log);
}

/* @} */

/**************************************************/ /**

 @name Log flusher thread

 *******************************************************/

/* @{ */

static void log_flush_update_stats(log_t &log) {
  ut_ad(log_flusher_mutex_own(log));

  /* Note that this code is inspired by similar logic in buf0flu.cc */

  static uint64_t iterations = 0;
  static Log_clock_point prev_time{};
  static lsn_t prev_lsn;
  static lsn_t lsn_avg_rate = 0;
  static Log_clock::duration fsync_max_time;
  static Log_clock::duration fsync_total_time;

  /* Calculate time of last fsync and update related counters. */

  Log_clock::duration fsync_time;

  fsync_time = log.last_flush_end_time - log.last_flush_start_time;

  ut_a(fsync_time.count() >= 0);

  fsync_max_time = std::max(fsync_max_time, fsync_time);

  fsync_total_time += fsync_time;

  MONITOR_INC_VALUE(
      MONITOR_LOG_FLUSH_TOTAL_TIME,
      std::chrono::duration_cast<std::chrono::milliseconds>(fsync_time)
          .count());

  /* Calculate time elapsed since start of last sample. */

  if (prev_time == Log_clock_point{}) {
    prev_time = log.last_flush_start_time;
    prev_lsn = log.flushed_to_disk_lsn.load();
  }

  const Log_clock_point curr_time = log.last_flush_end_time;

  if (curr_time < prev_time) {
    /* Time was moved backward since we set prev_time.
    We cannot determine how much time passed since then. */
    prev_time = curr_time;
  }

  auto time_elapsed =
      std::chrono::duration_cast<std::chrono::seconds>(curr_time - prev_time)
          .count();

  ut_a(time_elapsed >= 0);

  if (++iterations >= srv_flushing_avg_loops ||
      time_elapsed >= static_cast<double>(srv_flushing_avg_loops)) {
    if (time_elapsed < 1) {
      time_elapsed = 1;
    }

    const lsn_t curr_lsn = log.flushed_to_disk_lsn.load();

    const lsn_t lsn_rate = static_cast<lsn_t>(
        static_cast<double>(curr_lsn - prev_lsn) / time_elapsed);

    lsn_avg_rate = (lsn_avg_rate + lsn_rate) / 2;

    MONITOR_SET(MONITOR_LOG_FLUSH_LSN_AVG_RATE, lsn_avg_rate);

    MONITOR_SET(
        MONITOR_LOG_FLUSH_MAX_TIME,
        std::chrono::duration_cast<std::chrono::microseconds>(fsync_max_time)
            .count());

    log.flush_avg_time =
        std::chrono::duration_cast<std::chrono::microseconds>(fsync_total_time)
            .count() *
        1.0 / iterations;

    MONITOR_SET(MONITOR_LOG_FLUSH_AVG_TIME, log.flush_avg_time);

    fsync_max_time = Log_clock::duration{};
    fsync_total_time = Log_clock::duration{};
    iterations = 0;
    prev_time = curr_time;
    prev_lsn = curr_lsn;
  }
}

static void log_flush_low(log_t &log) {
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

void log_flusher(log_t *log_ptr) {
  ut_a(log_ptr != nullptr);
  ut_a(log_ptr->flusher_thread_alive.load());

  log_t &log = *log_ptr;

  Log_thread_waiting waiting{log, log.flusher_event, srv_log_flusher_spin_delay,
                             srv_log_flusher_timeout};

  log_flusher_mutex_enter(log);

  for (uint64_t step = 0; log.writer_thread_alive.load(); ++step) {
    bool released = false;

    auto stop_condition = [&log, &released, step](bool wait) {

      if (released) {
        log_flusher_mutex_enter(log);
        released = false;
      }

      LOG_SYNC_POINT("log_flusher_before_should_flush");

      const lsn_t last_flush_lsn = log.flushed_to_disk_lsn.load();

      ut_a(last_flush_lsn <= log.write_lsn.load());

      if (last_flush_lsn < log.write_lsn.load()) {
        /* Flush and stop waiting. */
        log_flush_low(log);

        if (step % 1024 == 0) {
          log_flusher_mutex_exit(log);

          os_thread_sleep(0);

          log_flusher_mutex_enter(log);
        }

        return (true);
      }

      /* Stop waiting if writer thread is dead. */
      if (!log.writer_thread_alive.load()) {
        return (true);
      }

      if (wait) {
        log_flusher_mutex_exit(log);
        released = true;
      }

      return (false);
    };

    if (srv_flush_log_at_trx_commit != 1) {
      const auto current_time = Log_clock::now();

      ut_ad(log.last_flush_end_time >= log.last_flush_start_time);

      if (current_time < log.last_flush_end_time) {
        /* Time was moved backward, possibly by a lot, so we need to
        adjust the last_flush times, because otherwise we could stop
        flushing every innodb_flush_log_at_timeout for a while. */
        log.last_flush_start_time = current_time;
        log.last_flush_end_time = current_time;
      }

      const auto time_elapsed = current_time - log.last_flush_start_time;

      using us = std::chrono::microseconds;

      const auto time_elapsed_us =
          std::chrono::duration_cast<us>(time_elapsed).count();

      ut_a(time_elapsed_us >= 0);

      const auto flush_every = srv_flush_log_at_timeout;

      const auto flush_every_us = 1000000LL * flush_every;

      if (time_elapsed_us < flush_every_us) {
        log_flusher_mutex_exit(log);

        if (log.writer_thread_alive.load()) {
          /* If log.writer_thread_alive became false just now, we would
          have the flusher_event set, because: the only place where we
          can reset is just before fsync and after writer_thread_alive
          is set to false, the flusher_event is set. */

          os_event_wait_time_low(log.flusher_event,
                                 flush_every_us - time_elapsed_us, 0);
        }

        log_flusher_mutex_enter(log);
      }
    }

    const auto wait_stats = waiting.wait(stop_condition);

    MONITOR_INC_WAIT_STATS(MONITOR_LOG_FLUSHER_, wait_stats);
  }

  if (log.write_lsn.load() > log.flushed_to_disk_lsn.load()) {
    log_flush_low(log);
  }

  log.flusher_thread_alive.store(false);

  os_event_set(log.flush_notifier_event);

  log_flusher_mutex_exit(log);
}



#endif /* !UNIV_HOTBACKUP */
