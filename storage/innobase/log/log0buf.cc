/*****************************************************************************

Copyright (c) 1995, 2019, Oracle and/or its affiliates. All Rights Reserved.

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

*****************************************************************************/

/**************************************************/ /**
 @file log/log0buf.cc

 Redo log buffer implementation, including functions to:

 -# Reserve space in the redo log buffer,

 -# Write to the reserved space in the log buffer,

 -# Add link to the log recent written buffer,

 -# Add link to the log recent closed buffer.

 *******************************************************/

#include <innodb/logger/info.h>
#include <innodb/align/ut_uint64_align_down.h>
#include <innodb/align/ut_uint64_align_up.h>
#include <innodb/wait/ut_wait_for.h>

#ifndef UNIV_HOTBACKUP

#include "arch0arch.h"
#include "log0log.h"
#include "log0recv.h"  /* recv_recovery_is_on() */
#include "srv0start.h" /* SRV_SHUTDOWN_FLUSH_PHASE */

/**************************************************/


/** Waits until there is free space in log buffer up to reserved handle.end_sn.
If there was no space, it basically waits for log writer thread which copies
data from log buffer to log files and advances log.write_lsn, reclaiming space
in the log buffer (it's a ring buffer).

There is a special case - if it turned out, that log buffer is too small for
the reserved range of lsn values, it resizes the log buffer.

It's used during reservation of lsn values, when the reserved handle.end_sn is
greater than log.sn_limit_for_end.

@param[in,out]	log		redo log
@param[in]	handle		handle for the reservation */
static void log_wait_for_space_after_reserving(log_t &log,
                                               const Log_handle &handle);


/**************************************************/ /**

 @name Reservation of space in the redo log

 *******************************************************/

/* @{ */

static void log_wait_for_space_after_reserving(log_t &log,
                                               const Log_handle &handle) {
  ut_ad(log.sn_lock.s_own(handle.lock_no));

  const sn_t start_sn = log_translate_lsn_to_sn(handle.start_lsn);

  const sn_t end_sn = log_translate_lsn_to_sn(handle.end_lsn);

  const sn_t len = end_sn - start_sn;

  /* If we had not allowed to resize log buffer, it would have
  been sufficient here to simply call:
          - log_wait_for_space_in_log_buf(log, end_sn).

  However we do allow, and we need to handle the possible race
  condition, when user tries to set very small log buffer size
  and other threads try to write large groups of log records.

  Note that since this point, log.buf_size_sn may only be
  increased from our point of view. That's because:

          1. Other threads doing mtr_commit will only try to
             increase the size (if needed).

          2. If user wanted to manually resize the log buffer,
             he needs to obtain x-lock for the redo log, but
             we keep s-lock. */

  log_wait_for_space_in_log_buf(log, start_sn);

  /* Now start_sn fits the log buffer or is at the boundary.
  Therefore all previous reservations (for smaller LSN), fit
  the log buffer [1].

  We check if len > log.buf_size_sn. If that's the case, our
  range start_sn..end_sn will cover more than size of the log
  buffer and we need to extend the size. Note that users that
  reserved smaller LSN will not try to extend because of [1].
  Users that reserved larger LSN, will not have their start_sn
  in the log buffer, because our end_sn already does not fit.
  Such users will first wait to reach invariant [1]. */

  LOG_SYNC_POINT("log_wfs_after_reserving_before_buf_size_1");

  if (len > log.buf_size_sn.load()) {
    DBUG_EXECUTE_IF("ib_log_buffer_is_short_crash", DBUG_SUICIDE(););

    log_write_up_to(log, log_translate_sn_to_lsn(start_sn), false);

    /* Now the whole log has been written to disk up to start_sn,
    so there are no pending writes to log buffer for smaller sn. */
    LOG_SYNC_POINT("log_wfs_after_reserving_before_buf_size_2");

    /* Reservations for larger LSN could not increase size of log
    buffer as they could not have reached [1], because end_sn did
    not fit the log buffer (end_sn - start_sn > buf_size_sn), and
    next reservations would have their start_sn even greater. */
    ut_a(len > log.buf_size_sn.load());

    /* Note that the log.write_lsn could not be changed since it
    reached start_sn, until current thread continues and finishes
    writing its data to the log buffer.

    Note that any other thread will not attempt to write
    concurrently to the log buffer, because the log buffer
    represents range of sn:
            [start_sn, start_sn + log.buf_size_sn)
    and it holds:
            end_sn > start_sn + log_buf_size_sn.
    This will not change until we finished resizing log
    buffer and updated log.buf_size_sn, which therefore
    must happen at the very end of the resize procedure. */
    ut_a(log_translate_lsn_to_sn(log.write_lsn.load()) == start_sn);

    ib::info(ER_IB_MSG_1231)
        << "The transaction log size is too large"
        << " for srv_log_buffer_size (" << len << " > "
        << log.buf_size_sn.load() << "). Trying to extend it.";

    /* Resize without extra locking required.

    We cannot call log_buffer_resize() because it would try
    to acquire x-lock for the redo log and we keep s-lock.

    We already have ensured, that there are no possible
    concurrent writes to the log buffer. Note, we have also
    ensured that log writer finished writing up to start_sn.

    However, for extra safety, we prefer to acquire writer_mutex,
    and checkpointer_mutex. We consider this rare event. */

    log_checkpointer_mutex_enter(log);
    log_writer_mutex_enter(log);

    /* We multiply size at least by 1.382 to avoid case
    in which we keep resizing by few bytes only. */

    lsn_t new_lsn_size = log_translate_sn_to_lsn(
        static_cast<lsn_t>(1.382 * len + OS_FILE_LOG_BLOCK_SIZE));

    new_lsn_size = ut_uint64_align_up(new_lsn_size, OS_FILE_LOG_BLOCK_SIZE);

    log_buffer_resize_low(log, new_lsn_size, handle.start_lsn);

    log_writer_mutex_exit(log);
    log_checkpointer_mutex_exit(log);

  } else {
    /* Note that the size cannot get decreased.
    We are safe to continue. */
  }

  ut_a(len <= log.buf_size_sn.load());

  log_wait_for_space_in_log_buf(log, end_sn);
}



void log_free_check_wait(log_t &log, sn_t sn) {
  auto stop_condition = [&log, sn](bool) {

    const sn_t margins = log_free_check_margin(log);

    const lsn_t start_lsn = log_translate_sn_to_lsn(sn + margins);

    const lsn_t checkpoint_lsn = log.last_checkpoint_lsn.load();

    if (start_lsn <= checkpoint_lsn + log.lsn_capacity_for_free_check) {
      /* No reason to wait anymore. */
      return (true);
    }

    log_request_checkpoint(log, true,
                           start_lsn - log.lsn_capacity_for_free_check);

    return (false);
  };

  const auto wait_stats = ut_wait_for(0, 100, stop_condition);

  MONITOR_INC_WAIT_STATS(MONITOR_LOG_ON_FILE_SPACE_, wait_stats);
}

void log_wait_for_space_in_log_buf(log_t &log, sn_t end_sn) {
  lsn_t lsn;
  Wait_stats wait_stats;

  const sn_t write_sn = log_translate_lsn_to_sn(log.write_lsn.load());

  LOG_SYNC_POINT("log_wait_for_space_in_buf_middle");

  const sn_t buf_size_sn = log.buf_size_sn.load();

  if (end_sn + OS_FILE_LOG_BLOCK_SIZE <= write_sn + buf_size_sn) {
    return;
  }

  /* We preserve this counter for backward compatibility with 5.7. */
  srv_stats.log_waits.inc();

  lsn = log_translate_sn_to_lsn(end_sn + OS_FILE_LOG_BLOCK_SIZE - buf_size_sn);

  wait_stats = log_write_up_to(log, lsn, false);

  MONITOR_INC_WAIT_STATS(MONITOR_LOG_ON_BUFFER_SPACE_, wait_stats);

  ut_a(end_sn + OS_FILE_LOG_BLOCK_SIZE <=
       log_translate_lsn_to_sn(log.write_lsn.load()) + buf_size_sn);
}

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

/* @} */

/**************************************************/ /**

 @name Writing to the redo log buffer

 *******************************************************/

/* @{ */

lsn_t log_buffer_write(log_t &log, const Log_handle &handle, const byte *str,
                       size_t str_len, lsn_t start_lsn) {
  ut_ad(log.sn_lock.s_own(handle.lock_no));

  ut_a(log.buf != nullptr);
  ut_a(log.buf_size > 0);
  ut_a(log.buf_size % OS_FILE_LOG_BLOCK_SIZE == 0);
  ut_a(str != nullptr);
  ut_a(str_len > 0);

  /* We should first resize the log buffer, if str_len is that big. */
  ut_a(str_len < log.buf_size_sn.load());

  /* The start_lsn points a data byte (not a header of log block). */
  ut_a(log_lsn_validate(start_lsn));

  /* We neither write with holes, nor overwrite any fragments of data. */
  ut_ad(log.write_lsn.load() <= start_lsn);
  ut_ad(log_buffer_ready_for_write_lsn(log) <= start_lsn);

  /* That's only used in the assertion at the very end. */
  const lsn_t end_sn = log_translate_lsn_to_sn(start_lsn) + str_len;

  /* A guard used to detect when we should wrap (to avoid overflowing
  outside the log buffer). */
  byte *buf_end = log.buf + log.buf_size;

  /* Pointer to next data byte to set within the log buffer. */
  byte *ptr = log.buf + (start_lsn % log.buf_size);

  /* Lsn value for the next byte to copy. */
  lsn_t lsn = start_lsn;

  /* Copy log records to the reserved space in the log buffer.
  Decrease number of bytes to copy (str_len) after some are
  copied. Proceed until number of bytes to copy reaches zero. */
  while (true) {
    /* Calculate offset from the beginning of log block. */
    const auto offset = lsn % OS_FILE_LOG_BLOCK_SIZE;

    ut_a(offset >= LOG_BLOCK_HDR_SIZE);
    ut_a(offset < OS_FILE_LOG_BLOCK_SIZE - LOG_BLOCK_TRL_SIZE);

    /* Calculate how many free data bytes are available
    within current log block. */
    const auto left = OS_FILE_LOG_BLOCK_SIZE - LOG_BLOCK_TRL_SIZE - offset;

    ut_a(left > 0);
    ut_a(left < OS_FILE_LOG_BLOCK_SIZE);

    size_t len, lsn_diff;

    if (left > str_len) {
      /* There are enough free bytes to finish copying
      the remaining part, leaving at least single free
      data byte in the log block. */

      len = str_len;

      lsn_diff = str_len;

    } else {
      /* We have more to copy than the current log block
      has remaining data bytes, or exactly the same.

      In both cases, next lsn value will belong to the
      next log block. Copy data up to the end of the
      current log block and start a next iteration if
      there is more to copy. */

      len = left;

      lsn_diff = left + LOG_BLOCK_TRL_SIZE + LOG_BLOCK_HDR_SIZE;
    }

    ut_a(len > 0);
    ut_a(ptr + len <= buf_end);

    LOG_SYNC_POINT("log_buffer_write_before_memcpy");

    /* This is the critical memcpy operation, which copies data
    from internal mtr's buffer to the shared log buffer. */
    std::memcpy(ptr, str, len);

    ut_a(len <= str_len);

    str_len -= len;
    str += len;
    lsn += lsn_diff;
    ptr += lsn_diff;

    ut_a(log_lsn_validate(lsn));

    if (ptr >= buf_end) {
      /* Wrap - next copy operation will write at the
      beginning of the log buffer. */

      ptr -= log.buf_size;
    }

    if (lsn_diff > left) {
      /* We have crossed boundaries between consecutive log
      blocks. Either we finish in next block, in which case
      user will set the proper first_rec_group field after
      this function is finished, or we finish even further,
      in which case next block should have 0. In both cases,
      we reset next block's value to 0 now, and in the first
      case, user will simply overwrite it afterwards. */

      ut_a((uintptr_t(ptr) % OS_FILE_LOG_BLOCK_SIZE) == LOG_BLOCK_HDR_SIZE);

      ut_a((uintptr_t(ptr) & ~uintptr_t(LOG_BLOCK_HDR_SIZE)) %
               OS_FILE_LOG_BLOCK_SIZE ==
           0);

      log_block_set_first_rec_group(
          reinterpret_cast<byte *>(uintptr_t(ptr) &
                                   ~uintptr_t(LOG_BLOCK_HDR_SIZE)),
          0);

      if (str_len == 0) {
        /* We have finished at the boundary. */
        break;
      }

    } else {
      /* Nothing more to copy - we have finished! */
      break;
    }
  }

  ut_a(ptr >= log.buf);
  ut_a(ptr <= buf_end);
  ut_a(buf_end == log.buf + log.buf_size);
  ut_a(log_translate_lsn_to_sn(lsn) == end_sn);

  return (lsn);
}

void log_buffer_write_completed(log_t &log, const Log_handle &handle,
                                lsn_t start_lsn, lsn_t end_lsn) {
  ut_ad(log.sn_lock.s_own(handle.lock_no));

  ut_a(log_lsn_validate(start_lsn));
  ut_a(log_lsn_validate(end_lsn));
  ut_a(end_lsn > start_lsn);

  /* Let M = log.recent_written_size (number of slots).
  For any integer k, all lsn values equal to: start_lsn + k*M
  correspond to the same slot, and only the smallest of them
  may use the slot. At most one of them can fit the range
  [log.buf_ready_for_write_lsn..log.buf_ready_ready_write_lsn+M).
  Any smaller values have already used the slot. Hence, we just
  need to wait until start_lsn will fit the mentioned range. */

  uint64_t wait_loops = 0;

  while (!log.recent_written.has_space(start_lsn)) {
    ++wait_loops;
    os_thread_sleep(20);
  }

  if (unlikely(wait_loops != 0)) {
    MONITOR_INC_VALUE(MONITOR_LOG_ON_RECENT_WRITTEN_WAIT_LOOPS, wait_loops);
  }

  /* Disallow reordering of writes to log buffer after this point.
  This is actually redundant, because we use seq_cst inside the
  log.recent_written.add_link(). However, we've decided to leave
  the seperate acq-rel synchronization between user threads and
  log writer. Reasons:
          1. Not to rely on internals of Link_buf::add_link.
          2. Stress that this synchronization is required in
             case someone decided to weaken memory ordering
             inside Link_buf. */
  std::atomic_thread_fence(std::memory_order_release);

  LOG_SYNC_POINT("log_buffer_write_completed_before_store");

  ut_ad(log.write_lsn.load() <= start_lsn);
  ut_ad(log_buffer_ready_for_write_lsn(log) <= start_lsn);

  /* Note that end_lsn will not point to just before footer,
  because we have already validated that end_lsn is valid. */
  log.recent_written.add_link(start_lsn, end_lsn);
}

void log_wait_for_space_in_log_recent_closed(log_t &log, lsn_t lsn) {
  ut_a(log_lsn_validate(lsn));

  ut_ad(lsn >= log_buffer_dirty_pages_added_up_to_lsn(log));

  uint64_t wait_loops = 0;

  while (!log.recent_closed.has_space(lsn)) {
    ++wait_loops;
    os_thread_sleep(20);
  }

  if (unlikely(wait_loops != 0)) {
    MONITOR_INC_VALUE(MONITOR_LOG_ON_RECENT_CLOSED_WAIT_LOOPS, wait_loops);
  }
}

void log_buffer_close(log_t &log, const Log_handle &handle) {
  const lsn_t start_lsn = handle.start_lsn;
  const lsn_t end_lsn = handle.end_lsn;

  ut_a(log_lsn_validate(start_lsn));
  ut_a(log_lsn_validate(end_lsn));
  ut_a(end_lsn > start_lsn);

  ut_ad(start_lsn >= log_buffer_dirty_pages_added_up_to_lsn(log));

  ut_ad(log.sn_lock.s_own(handle.lock_no));

  std::atomic_thread_fence(std::memory_order_release);

  LOG_SYNC_POINT("log_buffer_write_completed_dpa_before_store");

  log.recent_closed.add_link(start_lsn, end_lsn);

  log_buffer_s_lock_exit(log, handle.lock_no);
}

void log_buffer_set_first_record_group(log_t &log, const Log_handle &handle,
                                       lsn_t rec_group_end_lsn) {
  ut_ad(log.sn_lock.s_own(handle.lock_no));

  ut_a(log_lsn_validate(rec_group_end_lsn));

  const lsn_t last_block_lsn =
      ut_uint64_align_down(rec_group_end_lsn, OS_FILE_LOG_BLOCK_SIZE);

  byte *buf = log.buf;

  ut_a(buf != nullptr);

  byte *last_block_ptr = buf + (last_block_lsn % log.buf_size);

  LOG_SYNC_POINT("log_buffer_set_first_record_group_before_update");

  /* User thread needs to set proper first_rec_group value before
  link is added to recent written buffer. */
  ut_ad(log_buffer_ready_for_write_lsn(log) < rec_group_end_lsn);

  /* This also guarantees, that log buffer could not become resized
  mean while. */
  ut_a(buf + (last_block_lsn % log.buf_size) == last_block_ptr);

  /* This field is not overwritten. It is set to 0, when user thread
  crosses boundaries of consecutive log blocks. */
  ut_a(log_block_get_first_rec_group(last_block_ptr) == 0);

  log_block_set_first_rec_group(last_block_ptr,
                                rec_group_end_lsn % OS_FILE_LOG_BLOCK_SIZE);
}



void log_buffer_get_last_block(log_t &log, lsn_t &last_lsn, byte *last_block,
                               uint32_t &block_len) {
  ut_ad(last_block != nullptr);

  /* We acquire x-lock for the log buffer to prevent:
          a) resize of the log buffer
          b) overwrite of the fragment which we are copying */

  log_buffer_x_lock_enter(log);

  /* Because we have acquired x-lock for the log buffer, current
  lsn will not advance and all users that reserved smaller lsn
  have finished writing to the log buffer. */

  last_lsn = log_get_lsn(log);

  byte *buf = log.buf;

  ut_a(buf != nullptr);

  /* Copy last block from current buffer. */

  const lsn_t block_lsn =
      ut_uint64_align_down(last_lsn, OS_FILE_LOG_BLOCK_SIZE);

  byte *src_block = buf + block_lsn % log.buf_size;

  const auto data_len = last_lsn % OS_FILE_LOG_BLOCK_SIZE;

  ut_ad(data_len >= LOG_BLOCK_HDR_SIZE);

  /* The next_checkpoint_no is protected by the x-lock too. */

  const auto checkpoint_no = log.next_checkpoint_no.load();

  std::memcpy(last_block, src_block, data_len);

  /* We have copied data from the log buffer. We can release
  the x-lock and let new writes to the buffer go. Since now,
  we work only with our local copy of the data. */

  log_buffer_x_lock_exit(log);

  std::memset(last_block + data_len, 0x00, OS_FILE_LOG_BLOCK_SIZE - data_len);

  log_block_set_hdr_no(last_block, log_block_convert_lsn_to_no(block_lsn));

  log_block_set_data_len(last_block, data_len);

  ut_ad(log_block_get_first_rec_group(last_block) <= data_len);

  log_block_set_checkpoint_no(last_block, checkpoint_no);

  log_block_store_checksum(last_block);

  block_len = OS_FILE_LOG_BLOCK_SIZE;
}

/* @} */

/**************************************************/ /**

 @name Traversing links in the redo log recent buffers

 @todo Consider refactoring to extract common logic of
 two recent buffers to a common class (Links_buffer ?).

 *******************************************************/

/* @{ */

bool log_advance_ready_for_write_lsn(log_t &log) {
  ut_ad(log_writer_mutex_own(log));
  ut_d(log_writer_thread_active_validate(log));

  const lsn_t write_lsn = log.write_lsn.load();

  const auto write_max_size = srv_log_write_max_size;

  ut_a(write_max_size > 0);

  auto stop_condition = [&](lsn_t prev_lsn, lsn_t next_lsn) {

    ut_a(log_lsn_validate(prev_lsn));
    ut_a(log_lsn_validate(next_lsn));

    ut_a(next_lsn > prev_lsn);
    ut_a(prev_lsn >= write_lsn);

    LOG_SYNC_POINT("log_advance_ready_for_write_before_reclaim");

    return (prev_lsn - write_lsn >= write_max_size);
  };

  const lsn_t previous_lsn = log_buffer_ready_for_write_lsn(log);

  ut_a(previous_lsn >= write_lsn);

  if (log.recent_written.advance_tail_until(stop_condition)) {
    LOG_SYNC_POINT("log_advance_ready_for_write_before_update");

    /* Validation of recent_written is optional because
    it takes significant time (delaying the log writer). */
    if (log_test != nullptr &&
        log_test->enabled(Log_test::Options::VALIDATE_RECENT_WRITTEN)) {
      /* All links between ready_lsn and lsn have
      been traversed. The slots can't be re-used
      before we updated the tail. */
      log.recent_written.validate_no_links(previous_lsn,
                                           log_buffer_ready_for_write_lsn(log));
    }

    ut_a(log_buffer_ready_for_write_lsn(log) > previous_lsn);

    std::atomic_thread_fence(std::memory_order_acquire);

    return (true);

  } else {
    ut_a(log_buffer_ready_for_write_lsn(log) == previous_lsn);

    return (false);
  }
}

bool log_advance_dirty_pages_added_up_to_lsn(log_t &log) {
  ut_ad(log_closer_mutex_own(log));

  const lsn_t previous_lsn = log_buffer_dirty_pages_added_up_to_lsn(log);

  ut_a(previous_lsn >= LOG_START_LSN);

  ut_a(previous_lsn >= log_get_checkpoint_lsn(log));

  ut_d(log_closer_thread_active_validate(log));

  auto stop_condition = [&](lsn_t prev_lsn, lsn_t next_lsn) {

    ut_a(log_lsn_validate(prev_lsn));
    ut_a(log_lsn_validate(next_lsn));

    ut_a(next_lsn > prev_lsn);

    LOG_SYNC_POINT("log_advance_dpa_before_update");
    return (false);
  };

  if (log.recent_closed.advance_tail_until(stop_condition)) {
    LOG_SYNC_POINT("log_advance_dpa_before_reclaim");

    /* Validation of recent_closed is optional because
    it takes significant time (delaying the log closer). */
    if (log_test != nullptr &&
        log_test->enabled(Log_test::Options::VALIDATE_RECENT_CLOSED)) {
      /* All links between ready_lsn and lsn have
      been traversed. The slots can't be re-used
      before we updated the tail. */
      log.recent_closed.validate_no_links(
          previous_lsn, log_buffer_dirty_pages_added_up_to_lsn(log));
    }

    ut_a(log_buffer_dirty_pages_added_up_to_lsn(log) > previous_lsn);

    std::atomic_thread_fence(std::memory_order_acquire);

    return (true);

  } else {
    ut_a(log_buffer_dirty_pages_added_up_to_lsn(log) == previous_lsn);

    return (false);
  }
}

  /* @} */

#endif /* !UNIV_HOTBACKUP */
