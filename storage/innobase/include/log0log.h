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

#include <innodb/log_redo/log_files_header_fill.h>
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
#include <innodb/log_redo/log_files_real_offset_for_lsn.h>
#include <innodb/log_redo/log_files_real_offset.h>
#include <innodb/log_redo/log_files_size_offset.h>
#include <innodb/log_redo/meb_log_print_file_hdr.h>
#include <innodb/log_redo/log_buffer_x_lock_exit.h>
#include <innodb/log_redo/log_buffer_x_lock_enter.h>
#include <innodb/wait/Wait_stats.h>
#include <innodb/disk/flags.h>
#include <innodb/log_types/log_t.h>
#include <innodb/log_types/log_checksum_func_t.h>
#include <innodb/log_types/Log_handle.h>
#include <innodb/log_redo/flags.h>
#include <innodb/log_redo/log_sys.h>
#include <innodb/log_redo/log_checksum_algorithm_ptr.h>
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
#include <innodb/log_redo/log_block_calc_checksum.h>
#include <innodb/log_redo/log_block_calc_checksum_crc32.h>
#include <innodb/log_redo/log_block_calc_checksum_none.h>
#include <innodb/log_redo/log_block_get_checksum.h>
#include <innodb/log_redo/log_block_set_checksum.h>
#include <innodb/log_redo/log_block_store_checksum.h>
#include <innodb/log_redo/log_block_get_encrypt_bit.h>
#include <innodb/log_redo/log_block_set_encrypt_bit.h>
#include <innodb/log_redo/log_get_max_modified_age_async.h>
#include <innodb/log_redo/log_needs_free_check.h>
#include <innodb/log_redo/log_free_check.h>
#include <innodb/log_redo/log_translate_sn_to_lsn.h>
#include <innodb/log_redo/log_lsn_validate.h>
#include <innodb/log_redo/log_get_lsn.h>
#include <innodb/log_redo/log_get_checkpoint_lsn.h>
#include <innodb/log_redo/log_get_checkpoint_age.h>
#include <innodb/log_redo/log_buffer_flush_to_disk.h>
#include <innodb/log_redo/log_buffer_ready_for_write_lsn.h>
#include <innodb/log_redo/log_buffer_dirty_pages_added_up_to_lsn.h>
#include <innodb/log_redo/log_buffer_flush_order_lag.h>
#include <innodb/log_redo/log_write_to_file_requests_are_frequent.h>
#include <innodb/log_redo/log_buffer_s_lock_enter.h>
#include <innodb/log_redo/log_buffer_s_lock_exit.h>
#include <innodb/log_redo/LOG_SYNC_POINT.h>
#include <innodb/log_redo/Log_test.h>
#include <innodb/log_redo/log_test.h>



#include <innodb/machine/data.h>
#include "srv0mon.h"
#include "srv0srv.h"
#include <innodb/crc32/crc32.h>





#ifndef UNIV_HOTBACKUP



/* Declaration of log_buffer functions (definition in log0buf.cc). */

/** Reserves space in the redo log for following write operations.
Space is reserved for a given number of data bytes. Additionally
bytes for required headers and footers of log blocks are reserved.

After the space is reserved, range of lsn values from a start_lsn
to an end_lsn is assigned. The log writer thread cannot proceed
further than to the start_lsn, until a link start_lsn -> end_lsn
has been added to the log recent written buffer.

NOTE that the link is added after data is written to the reserved
space in the log buffer. It is very critical to do all these steps
as fast as possible, because very likely the log writer thread is
waiting for the link.

@see @ref sect_redo_log_buf_reserve
@param[in,out]	log	redo log
@param[in]	len	number of data bytes to reserve for write
@return handle that represents the reservation */
Log_handle log_buffer_reserve(log_t &log, size_t len);

/** Writes data to the log buffer. The space in the redo log has to be
reserved before calling to this function and lsn pointing to inside the
reserved range of lsn values has to be provided.

The write does not have to cover the whole reserved space, but may not
overflow it. If it does not cover, then returned value should be used
to start the next write operation. Note that finally we must use exactly
all the reserved space.

@see @ref sect_redo_log_buf_write
@param[in,out]	log		redo log
@param[in]	handle		handle for the reservation of space
@param[in]	str		memory to write data from
@param[in]	str_len		number of bytes to write
@param[in]	start_lsn	lsn to start writing at (the reserved space)

@return end_lsn after writing the data (in the reserved space), could be
used to start the next write operation if there is still free space in
the reserved space */
lsn_t log_buffer_write(log_t &log, const Log_handle &handle, const byte *str,
                       size_t str_len, lsn_t start_lsn);

/** Adds a link start_lsn -> end_lsn to the log recent written buffer.

This function must be called after the data has been written to the
fragment of log buffer represented by range [start_lsn, end_lsn).
After the link is added, the log writer may write the data to disk.

NOTE that still dirty pages for the [start_lsn, end_lsn) are not added
to flush lists when this function is called.

@see @ref sect_redo_log_buf_add_links_to_recent_written
@param[in,out]	log		redo log
@param[in]	handle		handle for the reservation of space
@param[in]	start_lsn	start_lsn of the link to add
@param[in]	end_lsn		end_lsn of the link to add */
void log_buffer_write_completed(log_t &log, const Log_handle &handle,
                                lsn_t start_lsn, lsn_t end_lsn);

/** Modifies header of log block in the log buffer, which contains
a given lsn value, and sets offset to the first group of log records
within the block.

This is used by mtr after writing a log record group which ends at
lsn belonging to different log block than lsn at which the group was
started. When write was finished at the last data byte of log block,
it is considered ended in the next log block, because the next data
byte belongs to that block.

During recovery, when recovery is started in the middle of some group
of log records, it first looks for the beginning of the next group.

@param[in,out]	log			redo log
@param[in]	handle			handle for the reservation of space
@param[in]	rec_group_end_lsn	lsn at which the first log record
group starts within the block containing this lsn value */
void log_buffer_set_first_record_group(log_t &log, const Log_handle &handle,
                                       lsn_t rec_group_end_lsn);

/** Adds a link start_lsn -> end_lsn to the log recent closed buffer.

This is called after all dirty pages related to [start_lsn, end_lsn)
have been added to corresponding flush lists.
For detailed explanation - @see log0write.cc.

@see @ref sect_redo_log_add_link_to_recent_closed
@param[in,out]	log		redo log
@param[in]	handle		handle for the reservation of space */
void log_buffer_close(log_t &log, const Log_handle &handle);










/** Get last redo block from redo buffer and end LSN. Note that it takes
x-lock on the log buffer for a short period. Out values are always set,
even when provided last_block is nullptr.
@param[in,out]	log		redo log
@param[out]	last_lsn	end lsn of last mtr
@param[out]	last_block	last redo block
@param[in,out]	block_len	length in bytes */
void log_buffer_get_last_block(log_t &log, lsn_t &last_lsn, byte *last_block,
                               uint32_t &block_len);

/** Advances log.buf_ready_for_write_lsn using links in the recent written
buffer. It's used by the log writer thread only.
@param[in]	log	redo log
@return true if and only if the lsn has been advanced */
bool log_advance_ready_for_write_lsn(log_t &log);

/** Advances log.buf_dirty_pages_added_up_to_lsn using links in the recent
closed buffer. It's used by the log closer thread only.
@param[in]	log	redo log
@return true if and only if the lsn has been advanced */
bool log_advance_dirty_pages_added_up_to_lsn(log_t &log);

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

/** Waits until there is free space in the log recent closed buffer
for any links start_lsn -> end_lsn, which start at provided start_lsn.
It does not add any link.

This is called just before dirty pages for [start_lsn, end_lsn)
are added to flush lists. That's because we need to guarantee,
that the delay until dirty page is added to flush list is limited.
For detailed explanation - @see log0write.cc.

@see @ref sect_redo_log_add_dirty_pages
@param[in,out]	log   redo log
@param[in]      lsn   lsn on which we wait (for any link: lsn -> x) */
void log_wait_for_space_in_log_recent_closed(log_t &log, lsn_t lsn);

/** Waits until there is free space in the log buffer. The free space has to be
available for range of sn values ending at the provided sn.
@see @ref sect_redo_log_waiting_for_writer
@param[in]     log     redo log
@param[in]     end_sn  end of the range of sn values */
void log_wait_for_space_in_log_buf(log_t &log, sn_t end_sn);

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



/** Waits until the redo log is written up to a provided lsn.
@param[in]  log             redo log
@param[in]  lsn             lsn to wait for
@param[in]  flush_to_disk   true: wait until it is flushed
@return statistics about waiting inside */
Wait_stats log_write_up_to(log_t &log, lsn_t lsn, bool flush_to_disk);

/* Read the first log file header to get the encryption
information if it exist.
@return true if success */
bool log_read_encryption();

/** Write the encryption info into the log file header(the 3rd block).
It just need to flush the file header block with current master key.
@param[in]	key	encryption key
@param[in]	iv	encryption iv
@param[in]	is_boot	if it is for bootstrap
@return true if success. */
bool log_write_encryption(byte *key, byte *iv, bool is_boot);

/** Rotate the redo log encryption
It will re-encrypt the redo log encryption metadata and write it to
redo log file header.
@return true if success. */
bool log_rotate_encryption();

/** Rotate default master key for redo log encryption. */
void redo_rotate_default_master_key();

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




/** Increase concurrency_margin used inside log_free_check() calls. */
void log_increase_concurrency_margin(log_t &log);





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





/** The log writer thread co-routine.
@see @ref sect_redo_log_writer
@param[in,out]	log_ptr		pointer to redo log */
void log_writer(log_t *log_ptr);

/** The log flusher thread co-routine.
@see @ref sect_redo_log_flusher
@param[in,out]	log_ptr		pointer to redo log */
void log_flusher(log_t *log_ptr);

/** The log flush notifier thread co-routine.
@see @ref sect_redo_log_flush_notifier
@param[in,out]	log_ptr		pointer to redo log */
void log_flush_notifier(log_t *log_ptr);

/** The log write notifier thread co-routine.
@see @ref sect_redo_log_write_notifier
@param[in,out]	log_ptr		pointer to redo log */
void log_write_notifier(log_t *log_ptr);

/** The log closer thread co-routine.
@see @ref sect_redo_log_closer
@param[in,out]	log_ptr		pointer to redo log */
void log_closer(log_t *log_ptr);

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
