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

#include <innodb/log_arch/log_archiver_thread_event.h>
#include <innodb/log_arch/arch_log_sys.h>
#include <innodb/log_write/log_writer_wait_on_checkpoint.h>
#include <innodb/log_write/log_flush_low.h>
#include <innodb/log_write/log_flush_update_stats.h>
#include <innodb/log_write/log_file_header_fill_encryption.h>
#include <innodb/log_write/log_wait_for_write.h>
#include <innodb/log_write/srv_log_wait_for_write_spin_delay.h>
#include <innodb/log_write/log_max_spins_when_waiting_in_user_thread.h>
#include <innodb/log_redo/log_closer_mutex_exit.h>
#include <innodb/log_redo/log_closer_mutex_enter.h>
#include <innodb/log_flush/log_flush_notifier_mutex_exit.h>
#include <innodb/log_flush/log_flush_notifier_mutex_enter.h>
#include <innodb/log_write/log_write_notifier_mutex_exit.h>
#include <innodb/log_write/log_write_notifier_mutex_enter.h>
#include <innodb/log_flush/log_flusher_mutex_exit.h>
#include <innodb/log_flush/log_flusher_mutex_enter.h>
#include <innodb/log_types/log_writer_mutex_enter.h>
#include <innodb/log_types/log_writer_mutex_exit.h>
#include <innodb/allocator/ut_malloc_nokey.h>
#include <innodb/logger/info.h>
#include <innodb/align/ut_uint64_align_up.h>
#include <innodb/align/ut_uint64_align_down.h>
#include <innodb/sync_event/os_event_wait_low.h>
#include <innodb/sync_event/os_event_reset.h>
#include <innodb/sync_event/os_event_wait_for.h>
#include <innodb/io/IORequestLogRead.h>
#include <innodb/io/IORequestLogWrite.h>
#include <innodb/log_files/log_files_header_flush.h>
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
#include <innodb/log_write/log_files_write_buffer.h>

#include "ha_prototypes.h"

#include <debug_sync.h>

#include "log0log.h"
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





/** Writes fragment of the log buffer up to provided lsn (not further).
Stops after the first call to fil_io() (possibly at smaller lsn).
Main side-effect: log.write_lsn is advanced.
@param[in]  log             redo log
@param[in]  next_write_lsn  write up to this lsn value */
static void log_writer_write_buffer(log_t &log, lsn_t next_write_lsn);




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

/* @} */


#endif /* !UNIV_HOTBACKUP */
