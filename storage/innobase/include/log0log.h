/*****************************************************************************

Copyright (c) 1995, 2019, Oracle and/or its affiliates. All rights reserved.
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
 @file include/log0log.h

 Redo log constants and functions.

 Types are defined inside log0types.h.

 Created 12/9/1995 Heikki Tuuri
 *******************************************************/

#ifndef log0log_h
#define log0log_h

#include <innodb/univ/univ.h>

#include <innodb/log_redo/log_buffer_set_first_record_group.h>
#include <innodb/log_redo/log_buffer_get_last_block.h>
#include <innodb/log_redo/log_advance_ready_for_write_lsn.h>
#include <innodb/log_write/redo_rotate_default_master_key.h>
#include <innodb/log_write/log_rotate_encryption.h>
#include <innodb/log_write/log_write_encryption.h>
#include <innodb/log_types/log_files_header_fill.h>
#include <innodb/log_redo/log_threads_active.h>
#include <innodb/log_redo/log_buffer_resize_low.h>
#include <innodb/log_redo/log_write_ahead_resize.h>
#include <innodb/log_redo/log_update_limits.h>
#include <innodb/log_redo/log_position_collect_lsn_info.h>
#include <innodb/log_redo/log_position_unlock.h>
#include <innodb/log_redo/log_position_lock.h>
#include <innodb/log_redo/log_refresh_stats.h>
#include <innodb/log_redo/log_print.h>
#include <innodb/log_redo/log_sys_close.h>
#include <innodb/log_redo/log_free_check_margin.h>
#include <innodb/log_redo/log_sys_init.h>
#include <innodb/log_redo/log_calc_max_ages.h>
#include <innodb/log_redo/log_calc_concurrency_margin.h>
#include <innodb/log_redo/log_writer_thread_active_validate.h>
#include <innodb/log_redo/log_closer_thread_active_validate.h>
#include <innodb/log_redo/log_background_write_threads_active_validate.h>
#include <innodb/log_redo/log_background_threads_active_validate.h>
#include <innodb/log_redo/log_background_threads_inactive_validate.h>
#include <innodb/log_redo/log_files_update_offsets.h>
#include <innodb/log_types/log_files_real_offset_for_lsn.h>
#include <innodb/log_types/log_files_real_offset.h>
#include <innodb/log_types/log_files_size_offset.h>
#include <innodb/log_redo/meb_log_print_file_hdr.h>
#include <innodb/log_types/log_buffer_x_lock_exit.h>
#include <innodb/log_types/log_buffer_x_lock_enter.h>
#include <innodb/wait/Wait_stats.h>
#include <innodb/disk/flags.h>
#include <innodb/log_types/log_t.h>
#include <innodb/log_types/log_checksum_func_t.h>
#include <innodb/log_types/Log_handle.h>
#include <innodb/log_types/flags.h>
#include <innodb/log_types/log_sys.h>
#include <innodb/log_types/log_checksum_algorithm_ptr.h>
#include <innodb/log_redo/log_allocate_buffer.h>
#include <innodb/log_redo/log_allocate_write_ahead_buffer.h>
#include <innodb/log_redo/log_allocate_checkpoint_buffer.h>
#include <innodb/log_redo/log_deallocate_checkpoint_buffer.h>
#include <innodb/log_redo/log_deallocate_flush_events.h>
#include <innodb/log_redo/log_allocate_flush_events.h>
#include <innodb/log_redo/log_deallocate_write_ahead_buffer.h>
#include <innodb/log_redo/log_deallocate_buffer.h>
#include <innodb/log_redo/log_translate_lsn_to_sn.h>
#include <innodb/log_redo/log_block_get_flush_bit.h>
#include <innodb/log_redo/log_block_set_flush_bit.h>
#include <innodb/log_redo/log_block_get_hdr_no.h>
#include <innodb/log_redo/log_block_set_hdr_no.h>
#include <innodb/log_redo/log_block_get_data_len.h>
#include <innodb/log_redo/log_block_set_data_len.h>
#include <innodb/log_redo/log_block_get_first_rec_group.h>
#include <innodb/log_redo/log_block_set_first_rec_group.h>
#include <innodb/log_redo/log_block_get_checkpoint_no.h>
#include <innodb/log_redo/log_block_set_checkpoint_no.h>
#include <innodb/log_redo/log_block_convert_lsn_to_no.h>
#include <innodb/log_types/log_block_calc_checksum.h>
#include <innodb/log_types/log_block_calc_checksum_crc32.h>
#include <innodb/log_types/log_block_calc_checksum_none.h>
#include <innodb/log_types/log_block_get_checksum.h>
#include <innodb/log_types/log_block_set_checksum.h>
#include <innodb/log_redo/log_block_store_checksum.h>
#include <innodb/log_redo/log_block_get_encrypt_bit.h>
#include <innodb/log_redo/log_block_set_encrypt_bit.h>
#include <innodb/log_redo/log_get_max_modified_age_async.h>
#include <innodb/log_redo/log_needs_free_check.h>
#include <innodb/log_redo/log_free_check.h>
#include <innodb/log_types/log_translate_sn_to_lsn.h>
#include <innodb/log_types/log_lsn_validate.h>
#include <innodb/log_types/log_get_lsn.h>
#include <innodb/log_types/log_get_checkpoint_lsn.h>
#include <innodb/log_redo/log_get_checkpoint_age.h>
#include <innodb/log_redo/log_buffer_flush_to_disk.h>
#include <innodb/log_redo/log_buffer_ready_for_write_lsn.h>
#include <innodb/log_redo/log_buffer_dirty_pages_added_up_to_lsn.h>
#include <innodb/log_redo/log_buffer_flush_order_lag.h>
#include <innodb/log_redo/log_write_to_file_requests_are_frequent.h>
#include <innodb/log_redo/log_buffer_s_lock_enter.h>
#include <innodb/log_redo/log_buffer_s_lock_exit.h>
#include <innodb/log_types/LOG_SYNC_POINT.h>
#include <innodb/log_types/Log_test.h>
#include <innodb/log_types/log_test.h>
#include <innodb/log_write/log_write_up_to.h>
#include <innodb/log_write/Log_write_to_file_requests_monitor.h>
#include <innodb/log_write/log_read_encryption.h>
#include <innodb/log_redo/log_advance_dirty_pages_added_up_to_lsn.h>
#include <innodb/log_redo/log_buffer_close.h>
#include <innodb/log_redo/log_wait_for_space_in_log_recent_closed.h>
#include <innodb/log_redo/log_buffer_write_completed.h>
#include <innodb/log_redo/log_buffer_write.h>
#include <innodb/log_write/log_wait_for_space_in_log_buf.h>
#include <innodb/log_write/log_wait_for_space_after_reserving.h>
#include <innodb/log_write/log_buffer_reserve.h>
#include <innodb/log_write/log_flush_notifier.h>
#include <innodb/log_write/Log_thread_waiting.h>
#include <innodb/log_write/log_flusher.h>
#include <innodb/log_write/log_writer.h>

#include <innodb/machine/data.h>
#include "srv0mon.h"
#include "srv0srv.h"
#include <innodb/crc32/crc32.h>





#ifndef UNIV_HOTBACKUP



/* Declaration of log_buffer functions (definition in log0buf.cc). */

















/** Validates that all slots in log recent written buffer for lsn values
in range between begin and end, are empty. Used during tests, crashes the
program if validation does not pass.
@param[in]	log		redo log which buffer is validated
@param[in]	begin		validation start (inclusive)
@param[in]	end		validation end (exclusive) */
void log_recent_written_empty_validate(const log_t &log, lsn_t begin,
                                       lsn_t end);

/** Validates that all slots in log recent closed buffer for lsn values
in range between begin and end, are empty. Used during tests, crashes the
program if validation does not pass.
@param[in]	log		redo log which buffer is validated
@param[in]	begin		validation start (inclusive)
@param[in]	end		validation end (exclusive) */
void log_recent_closed_empty_validate(const log_t &log, lsn_t begin, lsn_t end);

/* Declaration of remaining functions. */





/** Waits until there is free space for range of sn values ending
at the provided sn, in both the log buffer and in the log files.
@param[in]	log       redo log
@param[in]	end_sn    end of the range of sn values */
void log_wait_for_space(log_t &log, sn_t end_sn);



/** Waits until there is free space in log files which includes
concurrency margin required for all threads. You should rather
use log_free_check().
@see @ref sect_redo_log_reclaim_space
@param[in]	log   redo log
@param[in]	sn    sn for which there should be concurrency margin */
void log_free_check_wait(log_t &log, sn_t sn);













/** Requests a sharp checkpoint write for provided or greater lsn.
@param[in,out]	log	redo log
@param[in]	sync	true -> wait until it is finished
@param[in]  lsn   lsn for which we need checkpoint (or greater chkp) */
void log_request_checkpoint(log_t &log, bool sync, lsn_t lsn);

/** Requests a fuzzy checkpoint write (for lsn currently available
for checkpointing).
@param[in,out]	log	redo log
@param[in]	sync	true -> wait until it is finished */
void log_request_checkpoint(log_t &log, bool sync);

/** Make a checkpoint at the current lsn. Reads current lsn and waits
until all dirty pages have been flushed up to that lsn. Afterwards
requests a checkpoint write and waits until it is finished.
@param[in,out]	log	redo log
@return true iff current lsn was greater than last checkpoint lsn */
bool log_make_latest_checkpoint(log_t &log);

/** Make a checkpoint at the current lsn. Reads current lsn and waits
until all dirty pages have been flushed up to that lsn. Afterwards
requests a checkpoint write and waits until it is finished.
@return true iff current lsn was greater than last checkpoint lsn */
bool log_make_latest_checkpoint();

/** Reads a log file header page to log.checkpoint_buf.
@param[in,out]	log	redo log
@param[in]	header	0 or LOG_CHECKPOINT_1 or LOG_CHECKPOINT2 */
void log_files_header_read(log_t &log, uint32_t header);



/** Changes format of redo files to previous format version.

@note Note this will work between the two formats 5_7_9 & current because
the only change is the version number */
void log_files_downgrade(log_t &log);

/** Writes the next checkpoint info to header of the first log file.
Note that two pages of the header are used alternately for consecutive
checkpoints. If we crashed during the write, we would still have the
previous checkpoint info and recovery would work.
@param[in,out]	log			redo log
@param[in]	next_checkpoint_lsn	writes checkpoint at this lsn */
void log_files_write_checkpoint(log_t &log, lsn_t next_checkpoint_lsn);










#endif /* !UNIV_HOTBACKUP */






#ifndef UNIV_HOTBACKUP









/** Creates the first checkpoint ever in the log files. Used during
initialization of new log files. Flushes:
  - header of the first log file (including checkpoint headers),
  - log block with data addressed by the checkpoint lsn.
@param[in,out]	log	redo log
@param[in]	lsn	the first checkpoint lsn */
void log_create_first_checkpoint(log_t &log, lsn_t lsn);











/** Starts all the log background threads. This can be called only,
when the threads are inactive. This should never be called concurrently.
This may not be called during read-only mode.
@param[in,out]	log	redo log */
void log_start_background_threads(log_t &log);













/** The log checkpointer thread co-routine.
@see @ref sect_redo_log_checkpointer
@param[in,out]	log_ptr		pointer to redo log */
void log_checkpointer(log_t *log_ptr);










#else /* !UNIV_HOTBACKUP */

#ifdef UNIV_DEBUG

/** Print a log file header.
@param[in]	block	pointer to the log buffer */
void meb_log_print_file_hdr(byte *block);

#endif /* UNIV_DEBUG */

#endif /* !UNIV_HOTBACKUP */



#endif /* !log0log_h */
