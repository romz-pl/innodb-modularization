/*****************************************************************************

Copyright (c) 1995, 2019, Oracle and/or its affiliates. All Rights Reserved.
Copyright (c) 2009, Google Inc.

Portions of this file contain modifications contributed and copyrighted by
Google, Inc. Those modifications are gratefully acknowledged and are described
briefly in the InnoDB documentation. The contributions by Google are
incorporated with their permission, and subject to the conditions contained in
the file COPYING.Google.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2.0, as published by the
Free Software Foundation.

This program is also distributed with certain software (including but not
limited to OpenSSL) that is licensed under separate terms, as designated in a
particular file or component or in included license documentation. The authors
of MySQL hereby grant you an additional permission to link the program and
your derivative works with the separately licensed software that they have
included with MySQL.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License, version 2.0,
for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

*****************************************************************************/

/**************************************************/ /**
 @file log/log0log.cc


 Created 12/9/1995 Heikki Tuuri
 *******************************************************/

#include <innodb/allocator/aligned_pointer.h>
#include <innodb/logger/info.h>
#include <innodb/align/ut_uint64_align_up.h>
#include <innodb/align/ut_uint64_align_down.h>
#include <innodb/log_types/log_checksum_func_t.h>
#include <innodb/log_types/LSN_PF.h>
#include <innodb/log_types/log_header_format_t.h>
#include <innodb/log_redo/log_allocate_write_events.h>
#include <innodb/log_redo/log_deallocate_write_events.h>
#include <innodb/log_redo/log_allocate_recent_written.h>
#include <innodb/log_redo/log_deallocate_recent_written.h>
#include <innodb/log_redo/log_allocate_recent_closed.h>
#include <innodb/log_redo/log_deallocate_recent_closed.h>
#include <innodb/log_redo/log_allocate_file_header_buffers.h>
#include <innodb/log_redo/log_deallocate_file_header_buffers.h>
#include <innodb/log_redo/log_calc_buf_size.h>
#include <innodb/log_redo/log_sys_object.h>
#include <innodb/log_redo/pfs.h>

#ifndef UNIV_HOTBACKUP

#include <debug_sync.h>
#include <sys/types.h>
#include <time.h>
#include "dict0boot.h"
#include "ha_prototypes.h"
#include "os0thread-create.h"
#include "trx0sys.h"




/**************************************************/ /**

 @name	Start / stop of background threads

 *******************************************************/

/* @{ */


void log_start_background_threads(log_t &log) {
  ib::info(ER_IB_MSG_1258) << "Log background threads are being started...";

  std::atomic_thread_fence(std::memory_order_seq_cst);

  log_background_threads_inactive_validate(log);

  ut_ad(!log.disable_redo_writes);
  ut_a(!srv_read_only_mode);
  ut_a(log.sn.load() > 0);

  log.closer_thread_alive.store(true);
  log.checkpointer_thread_alive.store(true);
  log.writer_thread_alive.store(true);
  log.flusher_thread_alive.store(true);
  log.write_notifier_thread_alive.store(true);
  log.flush_notifier_thread_alive.store(true);

  log.should_stop_threads.store(false);

  std::atomic_thread_fence(std::memory_order_seq_cst);

  os_thread_create(log_checkpointer_thread_key, log_checkpointer, &log);

  os_thread_create(log_closer_thread_key, log_closer, &log);

  os_thread_create(log_writer_thread_key, log_writer, &log);

  os_thread_create(log_flusher_thread_key, log_flusher, &log);

  os_thread_create(log_write_notifier_thread_key, log_write_notifier, &log);

  os_thread_create(log_flush_notifier_thread_key, log_flush_notifier, &log);

  log_background_threads_active_validate(log);
}

void log_stop_background_threads(log_t &log) {
  /* We cannot stop threads when x-lock is acquired, because of scenario:
          * log_checkpointer starts log_checkpoint()
          * log_checkpoint() asks to persist dd dynamic metadata
          * dict_persist_dd_table_buffer() tries to write to redo
          * but cannot acquire shared lock on log.sn_lock
          * so log_checkpointer thread waits for this thread
            until the x-lock is released
          * but this thread waits until log background threads
            have been stopped - log_checkpointer is not stopped. */
  ut_ad(!log.sn_lock.x_own());

  ib::info(ER_IB_MSG_1259) << "Log background threads are being closed...";

  std::atomic_thread_fence(std::memory_order_seq_cst);

  log_background_threads_active_validate(log);

  ut_a(!srv_read_only_mode);

  log.should_stop_threads.store(true);

  /* Log writer may wait on writer_event with 100ms timeout, so we better
  wake him up, so he could notice that log.should_stop_threads has been
  set to true, finish his work and exit. */
  os_event_set(log.writer_event);

  /* The same applies to log_checkpointer thread and log_closer thread.
  However, it does not apply to others, because:
    - log_flusher monitors log.writer_thread_alive,
    - log_write_notifier monitors log.writer_thread_alive,
    - log_flush_notifier monitors log.flusher_thread_alive. */
  os_event_set(log.closer_event);
  os_event_set(log.checkpointer_event);

  /* Wait until threads are closed. */
  while (log.closer_thread_alive.load() ||
         log.checkpointer_thread_alive.load() ||
         log.writer_thread_alive.load() || log.flusher_thread_alive.load() ||
         log.write_notifier_thread_alive.load() ||
         log.flush_notifier_thread_alive.load()) {
    os_thread_sleep(100 * 1000);
  }

  std::atomic_thread_fence(std::memory_order_seq_cst);

  log_background_threads_inactive_validate(log);
}

bool log_threads_active(const log_t &log) {
  return (log.closer_thread_alive.load() ||
          log.checkpointer_thread_alive.load() ||
          log.writer_thread_alive.load() || log.flusher_thread_alive.load() ||
          log.write_notifier_thread_alive.load() ||
          log.flush_notifier_thread_alive.load());
}

/* @} */



/**************************************************/ /**

 @name	Resizing of buffers

 *******************************************************/

/* @{ */

bool log_buffer_resize_low(log_t &log, size_t new_size, lsn_t end_lsn) {
  ut_ad(log_checkpointer_mutex_own(log));
  ut_ad(log_writer_mutex_own(log));

  const lsn_t start_lsn =
      ut_uint64_align_down(log.write_lsn.load(), OS_FILE_LOG_BLOCK_SIZE);

  end_lsn = ut_uint64_align_up(end_lsn, OS_FILE_LOG_BLOCK_SIZE);

  if (end_lsn == start_lsn) {
    end_lsn += OS_FILE_LOG_BLOCK_SIZE;
  }

  ut_ad(end_lsn - start_lsn <= log.buf_size);

  if (end_lsn - start_lsn > new_size) {
    return (false);
  }

  /* Save the contents. */
  byte *tmp_buf = UT_NEW_ARRAY_NOKEY(byte, end_lsn - start_lsn);
  for (auto i = start_lsn; i < end_lsn; i += OS_FILE_LOG_BLOCK_SIZE) {
    std::memcpy(&tmp_buf[i - start_lsn], &log.buf[i % log.buf_size],
                OS_FILE_LOG_BLOCK_SIZE);
  }

  /* Re-allocate log buffer. */
  srv_log_buffer_size = static_cast<ulong>(new_size);
  log_deallocate_buffer(log);
  log_allocate_buffer(log);

  /* Restore the contents. */
  for (auto i = start_lsn; i < end_lsn; i += OS_FILE_LOG_BLOCK_SIZE) {
    std::memcpy(&log.buf[i % new_size], &tmp_buf[i - start_lsn],
                OS_FILE_LOG_BLOCK_SIZE);
  }
  UT_DELETE_ARRAY(tmp_buf);

  log_calc_buf_size(log);

  ut_a(srv_log_buffer_size == log.buf_size);

  ib::info(ER_IB_MSG_1260) << "srv_log_buffer_size was extended to "
                           << log.buf_size << ".";

  return (true);
}

bool log_buffer_resize(log_t &log, size_t new_size) {
  log_buffer_x_lock_enter(log);

  const lsn_t end_lsn = log_get_lsn(log);

  log_checkpointer_mutex_enter(log);
  log_writer_mutex_enter(log);

  const bool ret = log_buffer_resize_low(log, new_size, end_lsn);

  log_writer_mutex_exit(log);
  log_checkpointer_mutex_exit(log);
  log_buffer_x_lock_exit(log);

  return (ret);
}

void log_write_ahead_resize(log_t &log, size_t new_size) {
  ut_a(new_size >= INNODB_LOG_WRITE_AHEAD_SIZE_MIN);
  ut_a(new_size <= INNODB_LOG_WRITE_AHEAD_SIZE_MAX);

  log_writer_mutex_enter(log);

  log_deallocate_write_ahead_buffer(log);
  srv_log_write_ahead_size = static_cast<ulong>(new_size);

  log.write_ahead_end_offset =
      ut_uint64_align_down(log.write_ahead_end_offset, new_size);

  log_allocate_write_ahead_buffer(log);

  log_writer_mutex_exit(log);
}

void log_calc_buf_size(log_t &log) {
  ut_a(srv_log_buffer_size >= INNODB_LOG_BUFFER_SIZE_MIN);
  ut_a(srv_log_buffer_size <= INNODB_LOG_BUFFER_SIZE_MAX);

  log.buf_size = srv_log_buffer_size;

  /* The following update has to be the last operation during resize
  procedure of log buffer. That's because since this moment, possibly
  new concurrent writes for higher sn will start (which were waiting
  for free space in the log buffer). */

  log.buf_size_sn = log_translate_lsn_to_sn(log.buf_size);

  log_update_limits(log);
}

/* @} */



#endif /* !UNIV_HOTBACKUP */
