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
 @file log/log0chkp.cc

 Redo log checkpoints, margins and log file headers.

 File consists of four groups:
   1. Coordination between log and buffer pool (oldest_lsn)
   2. Reading/writing log file headers
   3. Making checkpoints
   4. Margin calculations

 *******************************************************/

#include <innodb/io/srv_read_only_mode.h>
#include <innodb/log_chkp/log_checkpoint_time_elapsed.h>
#include <innodb/log_arch/arch_page_sys.h>
#include <innodb/log_chkp/log_checkpointer_mutex_exit.h>
#include <innodb/log_chkp/log_checkpointer_mutex_enter.h>
#include <innodb/log_types/log_writer_mutex_enter.h>
#include <innodb/log_types/log_writer_mutex_exit.h>
#include <innodb/align/ut_uint64_align_down.h>
#include <innodb/align/ut_uint64_align_up.h>
#include <innodb/wait/ut_wait_for.h>
#include <innodb/sync_event/os_event_reset.h>
#include <innodb/sync_event/os_event_wait_low.h>
#include <innodb/sync_event/os_event_wait_time_low.h>
#include <innodb/io/IORequestLogRead.h>
#include <innodb/io/IORequestLogWrite.h>
#include <innodb/io/srv_is_being_started.h>
#include <innodb/log_types/log_header_format_t.h>
#include <innodb/log_types/recv_recovery_is_on.h>
#include <innodb/log_files/log_files_header_flush.h>
#include <innodb/log_chkp/log_should_checkpoint.h>

#include "ha_prototypes.h"

#ifndef UNIV_HOTBACKUP
#include <debug_sync.h>
#endif /* !UNIV_HOTBACKUP */

#include "log0log.h"
#include "buf0buf.h"
#include "buf0flu.h"
#include "dict0boot.h"
#include "dict0stats_bg.h"
#include "fil0fil.h"
#include "log0log.h"
#include "log0recv.h"

#include "srv0mon.h"
#include "srv0srv.h"
#include "srv0start.h"



#include <innodb/enum/to_int.h>


#ifndef UNIV_HOTBACKUP


/** Considers writing next checkpoint. Checks if checkpoint should be written
(using log_should_checkpoint()) and writes the checkpoint if that's the case.
@return true if checkpoint has been written */
static bool log_consider_checkpoint(log_t &log);

/** Considers requesting page cleaners to execute sync flush.
@return true if request has been made */
static bool log_consider_sync_flush(log_t &log);

/** Makes a checkpoint. Note that this function does not flush dirty blocks
from the buffer pool. It only checks what is lsn of the oldest modification
in the buffer pool, and writes information about the lsn in log files.
@param[in,out]	log	redo log */
static void log_checkpoint(log_t &log);



/** Requests a checkpoint written for lsn greater or equal to provided one.
The log.checkpointer_mutex has to be acquired before it is called, and it
is not released within this function.
@param[in,out]	log		redo log
@param[in]	requested_lsn	provided lsn (checkpoint should be not older) */
static void log_request_checkpoint_low(log_t &log, lsn_t requested_lsn);

/** Waits for checkpoint advanced to at least that lsn.
@param[in]	log	redo log
@param[in]	lsn	lsn up to which we are waiting */
static void log_wait_for_checkpoint(const log_t &log, lsn_t lsn);

/** Requests to flush dirty pages, to advance oldest_lsn
in flush lists to provided value. Wakes up page cleaners
and waits until the flush is finished.
@param[in]	log		redo log
@param[in]	new_oldest	expected oldest_lsn */
static void log_preflush_pool_modified_pages(const log_t &log,
                                             lsn_t new_oldest);

/**************************************************/ /**

 @name Coordination with buffer pool and oldest_lsn

 *******************************************************/

/* @{ */

/** Calculates lsn at which we might write a next checkpoint. It does the
best effort, but possibly the maximum allowed lsn, could be even bigger.
That's because the order of dirty pages in flush lists has been relaxed,
and we don't want to spend time on traversing the whole flush lists here.

Note that some flush lists could be empty, and some additions of dirty pages
could be pending (threads have written data to the log buffer and became
scheduled out just before adding the dirty pages). That's why the calculated
value cannot be larger than the log.buf_dirty_pages_added_up_to_lsn (only up
to this lsn value we are sure, that all the dirty pages have been added).

It is guaranteed, that the returned value will not be smaller than
the log.last_checkpoint_lsn.

@return lsn for which we might write the checkpoint */
static lsn_t log_get_available_for_checkpoint_lsn(const log_t &log) {
  ut_ad(log_checkpointer_mutex_own(log));

  /* The log_buffer_dirty_pages_added_up_to_lsn() can only increase,
  and that happens only after all related dirty pages have been added
  to the flush lists.

  Hence, to avoid issues related to race conditions, we follow order:

          1. Note lsn up to which all dirty pages have already been
             added to flush lists.

          2. Check buffer pool to get LWM lsn for unflushed dirty pages
             added to flush lists.

          3. Flush lists were empty (no LWM) => use [1] as LWM.

          4. Checkpoint LSN could be min(LWM, flushed_to_disk_lsn). */

  LOG_SYNC_POINT("log_get_available_for_chkp_lsn_before_dpa");

  const lsn_t dpa_lsn = log_buffer_dirty_pages_added_up_to_lsn(log);

  ut_a(dpa_lsn >= log.last_checkpoint_lsn.load());

  LOG_SYNC_POINT("log_get_available_for_chkp_lsn_before_buf_pool");

  lsn_t lwm_lsn = buf_pool_get_oldest_modification_lwm();

  /* Empty flush list. */

  if (lwm_lsn == 0) {
    /* There are no dirty pages. We cannot return current lsn,
    because some mtr's commit could be in the middle, after
    its log records have been written to log buffer, but before
    its dirty pages have been added to flush lists. */

    lwm_lsn = dpa_lsn;

  } else {
    /* Cannot go beyound dpa_lsn.

    Note that log_closer might still not advance dpa enough,
    because it might be scheduled out. Yes, the returned lwm
    guarantees it is able to advance, but it needs to do it! */

    lwm_lsn = std::min(lwm_lsn, dpa_lsn);
  }

  /* Cannot go beyond flushed lsn.

  We cannot write checkpoint at higher lsn than lsn up to which
  redo is flushed to disk. We must not wait for log writer/flusher
  in log_checkpoint(). Therefore we need to limit lsn for checkpoint.
  That's because we would risk a deadlock otherwise - because writer
  waits for advanced checkpoint, when it detected that there is no
  free space in log files.

  However, note that the deadlock would happen only if we created
  log records without dirty pages (during page flush we anyway wait
  for redo flushed up to page's newest_modification). */

  const lsn_t flushed_lsn = log.flushed_to_disk_lsn.load();

  lsn_t lsn = std::min(lwm_lsn, flushed_lsn);

  /* We expect in recovery that checkpoint_lsn is within data area
  of log block. In future we could get rid of this assumption, but
  we would need to ensure that recovery handles that properly.

  For that we would better refactor log0recv.cc and seperate two
  phases:
          1. Looking for the proper mtr boundary to start at (only parse).
          2. Actual parsing and applying changes. */

  if (lsn % OS_FILE_LOG_BLOCK_SIZE == 0) {
    /* Do not make checkpoints at block boundary.

    We hopefully will get rid of this exception and allow
    recovery to start at arbitrary checkpoint value. */
    lsn = lsn - OS_FILE_LOG_BLOCK_SIZE + LOG_BLOCK_HDR_SIZE;
  }

  ut_a(lsn % OS_FILE_LOG_BLOCK_SIZE >= LOG_BLOCK_HDR_SIZE);

  ut_a(lsn % OS_FILE_LOG_BLOCK_SIZE <
       OS_FILE_LOG_BLOCK_SIZE - LOG_BLOCK_TRL_SIZE);

  lsn = std::max(lsn, log.last_checkpoint_lsn.load());

  ut_a(lsn >= log.last_checkpoint_lsn.load());
  ut_a(lsn <= log.flushed_to_disk_lsn.load());

  return (lsn);
}

/** Calculates and updates lsn at which we might write a next checkpoint. */
static void log_update_available_for_checkpoint_lsn(log_t &log) {
  ut_ad(log_checkpointer_mutex_own(log));

  /* Update lsn available for checkpoint. */
  const lsn_t oldest_lsn = log_get_available_for_checkpoint_lsn(log);

  /* oldest_lsn can decrease in case previously buffer pool flush lists
  were empty and now a new dirty page appeared, which causes a maximum
  delay of log.recent_closed_size being suddenly subtracted. */

  if (oldest_lsn > log.available_for_checkpoint_lsn) {
    log.available_for_checkpoint_lsn = oldest_lsn;
  }
}

/* @} */

/**************************************************/ /**

 @name Log file headers

 *******************************************************/

/* @{ */





#endif /* UNIV_HOTBACKUP */
#ifdef UNIV_HOTBACKUP

#ifdef UNIV_DEBUG

/** Print a log file header.
@param[in]     block   pointer to the log buffer */
void meb_log_print_file_hdr(byte *block) {
  ib::info(ER_IB_MSG_1232) << "Log file header:"
                           << " format "
                           << mach_read_from_4(block + LOG_HEADER_FORMAT)
                           << " pad1 "
                           << mach_read_from_4(block + LOG_HEADER_PAD1)
                           << " start_lsn "
                           << mach_read_from_8(block + LOG_HEADER_START_LSN)
                           << " creator '" << block + LOG_HEADER_CREATOR << "'"
                           << " checksum " << log_block_get_checksum(block);
}

#endif /* UNIV_DEBUG */

#endif /* UNIV_HOTBACKUP */

#ifndef UNIV_HOTBACKUP


/* @} */

/**************************************************/ /**

 @name Making checkpoints

 *******************************************************/

/* @{ */

static lsn_t log_determine_checkpoint_lsn(log_t &log) {
  const lsn_t oldest_lsn = log.available_for_checkpoint_lsn;
  const lsn_t dict_lsn = log.dict_max_allowed_checkpoint_lsn;

  ut_a(dict_lsn == 0 || dict_lsn >= log.last_checkpoint_lsn);

  if (dict_lsn == 0) {
    return (oldest_lsn);
  } else {
    return (std::min(oldest_lsn, dict_lsn));
  }
}


static void log_checkpoint(log_t &log) {
  ut_ad(log_checkpointer_mutex_own(log));
  ut_a(!srv_read_only_mode);
  ut_ad(!srv_checkpoint_disabled);

  const lsn_t checkpoint_lsn = log_determine_checkpoint_lsn(log);

  if (arch_page_sys != nullptr) {
    arch_page_sys->flush_at_checkpoint(checkpoint_lsn);
  }

  LOG_SYNC_POINT("log_before_checkpoint_data_flush");

#ifdef _WIN32
  switch (srv_win_file_flush_method) {
    case SRV_WIN_IO_UNBUFFERED:
      break;
    case SRV_WIN_IO_NORMAL:
      fil_flush_file_spaces(to_int(FIL_TYPE_TABLESPACE));
      break;
  }
#else  /* !_WIN32 */
  switch (srv_unix_file_flush_method) {
    case SRV_UNIX_NOSYNC:
      break;
    case SRV_UNIX_O_DSYNC:
      /* O_SYNC is respected only for redo files and we need to
      flush data files here. For details look inside os0file.cc. */
    case SRV_UNIX_FSYNC:
    case SRV_UNIX_LITTLESYNC:
    case SRV_UNIX_O_DIRECT:
    case SRV_UNIX_O_DIRECT_NO_FSYNC:
      fil_flush_file_spaces(to_int(FIL_TYPE_TABLESPACE));
  }
#endif /* _WIN32 */

  if (log_test != nullptr) {
    log_test->fsync_written_pages();
  }

  ut_a(checkpoint_lsn >= log.last_checkpoint_lsn.load());

  ut_a(checkpoint_lsn <= log_buffer_dirty_pages_added_up_to_lsn(log));

#ifdef UNIV_DEBUG
  if (checkpoint_lsn > log.flushed_to_disk_lsn.load()) {
    /* We need log_flusher, because we need redo flushed up
    to the oldest_lsn, and it's not been flushed yet. */

    log_background_threads_active_validate(log);
  }
#endif

  ut_a(log.flushed_to_disk_lsn.load() >= checkpoint_lsn);

  const auto current_time = std::chrono::high_resolution_clock::now();

  log.last_checkpoint_time = current_time;

  DBUG_PRINT("ib_log", ("Starting checkpoint at " LSN_PF, checkpoint_lsn));

  log_files_write_checkpoint(log, checkpoint_lsn);

  DBUG_PRINT("ib_log",
             ("checkpoint ended at " LSN_PF ", log flushed to " LSN_PF,
              log.last_checkpoint_lsn.load(), log.flushed_to_disk_lsn.load()));

  MONITOR_INC(MONITOR_LOG_CHECKPOINTS);

  DBUG_EXECUTE_IF("crash_after_checkpoint", DBUG_SUICIDE(););
}

void log_create_first_checkpoint(log_t &log, lsn_t lsn) {
  byte block[OS_FILE_LOG_BLOCK_SIZE];
  lsn_t block_lsn;
  page_no_t block_page_no;
  uint64_t block_offset;

  ut_a(srv_is_being_started);
  ut_a(!srv_read_only_mode);
  ut_a(!recv_recovery_is_on());
  ut_a(buf_are_flush_lists_empty_validate());

  log_background_threads_inactive_validate(log);

  /* Write header of first file. */
  log_files_header_flush(*log_sys, 0, LOG_START_LSN);

  /* Write header in log file which is responsible for provided lsn. */
  block_lsn = ut_uint64_align_down(lsn, OS_FILE_LOG_BLOCK_SIZE);

  block_offset = log_files_real_offset_for_lsn(log, block_lsn);

  uint32_t nth_file = static_cast<uint32_t>(block_offset / log.file_size);
  log_files_header_flush(log, nth_file, block_lsn);

  /* Write the first, empty log block. */
  std::memset(block, 0x00, OS_FILE_LOG_BLOCK_SIZE);
  log_block_set_hdr_no(block, log_block_convert_lsn_to_no(block_lsn));
  log_block_set_flush_bit(block, true);
  log_block_set_data_len(block, LOG_BLOCK_HDR_SIZE);
  log_block_set_checkpoint_no(block, 0);
  log_block_set_first_rec_group(block, 0);
  log_block_store_checksum(block);

  block_page_no =
      static_cast<page_no_t>(block_offset / univ_page_size.physical());

  auto err = fil_redo_io(
      IORequestLogWrite, page_id_t{log.files_space_id, block_page_no},
      univ_page_size, static_cast<ulint>(block_offset % UNIV_PAGE_SIZE),
      OS_FILE_LOG_BLOCK_SIZE, block);

  ut_a(err == DB_SUCCESS);

  /* Start writing the checkpoint. */
  log.last_checkpoint_lsn.store(0);
  log.next_checkpoint_no.store(0);
  log_files_write_checkpoint(log, lsn);

  /* Note, that checkpoint was responsible for fsync of all log files. */
}

static void log_request_checkpoint_low(log_t &log, lsn_t requested_lsn) {
  ut_a(requested_lsn < LSN_MAX);
  ut_ad(log_checkpointer_mutex_own(log));

#ifdef UNIV_DEBUG
  if (srv_checkpoint_disabled) {
    /* Checkpoints are disabled. Pretend it succeeded. */
    ib::info(ER_IB_MSG_1233) << "Checkpoint explicitly disabled!";
    return;
  }
#endif /* UNIV_DEBUG */

  ut_a(requested_lsn % OS_FILE_LOG_BLOCK_SIZE >= LOG_BLOCK_HDR_SIZE);

  ut_a(requested_lsn % OS_FILE_LOG_BLOCK_SIZE <
       OS_FILE_LOG_BLOCK_SIZE - LOG_BLOCK_TRL_SIZE);

  requested_lsn = std::max(requested_lsn, log.last_checkpoint_lsn.load());

  /* Update log.requested_checkpoint_lsn only to greater value. */

  if (requested_lsn > log.requested_checkpoint_lsn) {
    log.requested_checkpoint_lsn = requested_lsn;

    os_event_set(log.checkpointer_event);
  }
}

static void log_wait_for_checkpoint(const log_t &log, lsn_t requested_lsn) {
  log_background_threads_active_validate(log);

  auto stop_condition = [&log, requested_lsn](bool) {

    return (log.last_checkpoint_lsn.load() >= requested_lsn);
  };

  ut_wait_for(0, 100, stop_condition);
}

void log_request_checkpoint(log_t &log, bool sync, lsn_t lsn) {
  log_preflush_pool_modified_pages(log, lsn);

  log_checkpointer_mutex_enter(log);

  log_request_checkpoint_low(log, lsn);

  log_checkpointer_mutex_exit(log);

  if (sync) {
    log_wait_for_checkpoint(log, lsn);
  }
}

void log_request_checkpoint(log_t &log, bool sync) {
  log_checkpointer_mutex_enter(log);

  log_update_available_for_checkpoint_lsn(log);

  const lsn_t lsn = log.available_for_checkpoint_lsn;

  log_request_checkpoint_low(log, lsn);

  log_checkpointer_mutex_exit(log);

  if (sync) {
    log_wait_for_checkpoint(log, lsn);
  }
}

bool log_make_latest_checkpoint(log_t &log) {
  const lsn_t lsn = log_get_lsn(log);

  log_preflush_pool_modified_pages(log, lsn);

  log_checkpointer_mutex_enter(log);

  if (lsn <= log.last_checkpoint_lsn.load()) {
    log_checkpointer_mutex_exit(log);
    return (false);
  }

  log_request_checkpoint_low(log, lsn);

  log_checkpointer_mutex_exit(log);

  log_wait_for_checkpoint(log, lsn);

  return (true);
}

bool log_make_latest_checkpoint() {
  return (log_make_latest_checkpoint(*log_sys));
}

static void log_preflush_pool_modified_pages(const log_t &log,
                                             lsn_t new_oldest) {
  if (log_test != nullptr) {
    return;
  }

  /* A flush is urgent: we have to do a synchronous flush,
  because the oldest dirty page is too old.

  Note, that this could fire even if we did not run out
  of space in log files (users still may write to redo). */

  if (new_oldest == LSN_MAX
      /* Forced flush request is processed by page_cleaner, if
      it's not active, then we must do flush ourselves. */
      || !buf_page_cleaner_is_active
      /* Reason unknown. */
      || srv_is_being_started) {
    buf_flush_sync_all_buf_pools();

  } else {
    new_oldest += log_buffer_flush_order_lag(log);

    /* Wake up page cleaner asking to perform sync flush
    (unless user explicitly disabled sync-flushes). */
    if (srv_flush_sync) {
      buf_flush_request_force(new_oldest);
    }
  }
}

static bool log_consider_sync_flush(log_t &log) {
  ut_ad(log_checkpointer_mutex_own(log));

  const lsn_t oldest_lsn = log.available_for_checkpoint_lsn;

  lsn_t current_lsn = log_get_lsn(log);

  lsn_t flush_up_to = oldest_lsn;

  ut_a(oldest_lsn <= current_lsn);

  if (current_lsn == oldest_lsn) {
    return (false);
  }

  const sn_t margin = log_free_check_margin(log);

  current_lsn =
      log_translate_sn_to_lsn(log_translate_lsn_to_sn(current_lsn) + margin);

  if (current_lsn - oldest_lsn > log.max_modified_age_sync) {
    ut_a(current_lsn > log.max_modified_age_sync);

    flush_up_to = current_lsn - log.max_modified_age_sync;
  }

  const lsn_t requested_checkpoint_lsn = log.requested_checkpoint_lsn;

  if (requested_checkpoint_lsn > flush_up_to) {
    flush_up_to = requested_checkpoint_lsn;
  }

  if (flush_up_to > oldest_lsn) {
    log_checkpointer_mutex_exit(log);

    log_preflush_pool_modified_pages(log, flush_up_to);

    log_checkpointer_mutex_enter(log);

    /* It's very probable that forced flush will result in maximum
    lsn available for creating a new checkpoint, just try to update
    it to not wait for next checkpointer loop. */
    log_update_available_for_checkpoint_lsn(log);

    return (true);
  }

  return (false);
}




static bool log_consider_checkpoint(log_t &log) {
  ut_ad(log_checkpointer_mutex_own(log));

  if (!log_should_checkpoint(log)) {
    return (false);
  }

  /* It's clear that a new checkpoint should be written.
  So do write back the dynamic metadata. Since the checkpointer
  mutex is low-level one, it has to be released first. */
  log_checkpointer_mutex_exit(log);

  if (log_test == nullptr) {
    dict_persist_to_dd_table_buffer();
  }

  log_checkpointer_mutex_enter(log);

  /* We need to re-check if checkpoint should really be
  written, because we re-acquired the checkpointer_mutex.
  Some conditions could have changed - e.g. user could
  acquire the mutex and specify srv_checkpoint_disabled=T.
  Instead of trying to figure out which conditions could
  have changed, we follow a simple way and perform a full
  re-check of all conditions. */
  if (!log_should_checkpoint(log)) {
    return (false);
  }

  log_checkpoint(log);

  return (true);
}

void log_checkpointer(log_t *log_ptr) {
  ut_a(log_ptr != nullptr);
  ut_a(log_ptr->checkpointer_thread_alive.load());

  log_t &log = *log_ptr;

  log_checkpointer_mutex_enter(log);

  while (true) {
    auto do_some_work = [&log] {

      ut_ad(log_checkpointer_mutex_own(log));

      /* We will base our next decisions on maximum lsn
      available for creating a new checkpoint. It would
      be great to have it updated beforehand. Also, this
      is the only thread that relies on that value, so we
      don't need to update it in other threads. */
      log_update_available_for_checkpoint_lsn(log);

      /* Consider flushing some dirty pages. */
      const bool sync_flushed = log_consider_sync_flush(log);

      LOG_SYNC_POINT("log_checkpointer_before_consider_checkpoint");

      /* Consider writing checkpoint. */
      const bool checkpointed = log_consider_checkpoint(log);

      if (sync_flushed || checkpointed) {
        return (true);
      }

      return (false);
    };

    const auto sig_count = os_event_reset(log.checkpointer_event);

    if (!do_some_work()) {
      log_checkpointer_mutex_exit(log);

      os_event_wait_time_low(log.checkpointer_event, 10 * 1000, sig_count);

      log_checkpointer_mutex_enter(log);

    } else {
      log_checkpointer_mutex_exit(log);

      os_thread_sleep(0);

      log_checkpointer_mutex_enter(log);
    }

    /* Check if we should close the thread. */
    if (log.should_stop_threads.load() && !log.closer_thread_alive.load()) {
      break;
    }
  }

  log.checkpointer_thread_alive.store(false);

  log_checkpointer_mutex_exit(log);
}

/* @} */


#endif /* !UNIV_HOTBACKUP */
