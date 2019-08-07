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
#include <innodb/log_redo/log_buffer_x_lock_own.h>
#include <innodb/log_redo/log_checkpointer_mutex_enter.h>
#include <innodb/log_redo/log_checkpointer_mutex_exit.h>
#include <innodb/log_redo/log_checkpointer_mutex_own.h>
#include <innodb/log_redo/log_closer_mutex_enter.h>
#include <innodb/log_redo/log_closer_mutex_exit.h>
#include <innodb/log_redo/log_closer_mutex_own.h>
#include <innodb/log_redo/log_flusher_mutex_enter.h>
#include <innodb/log_redo/log_flusher_mutex_enter_nowait.h>
#include <innodb/log_redo/log_flusher_mutex_exit.h>
#include <innodb/log_redo/log_flusher_mutex_own.h>
#include <innodb/log_redo/log_flush_notifier_mutex_enter.h>
#include <innodb/log_redo/log_flush_notifier_mutex_exit.h>
#include <innodb/log_redo/log_flush_notifier_mutex_own.h>
#include <innodb/log_redo/log_writer_mutex_enter.h>
#include <innodb/log_redo/log_writer_mutex_enter_nowait.h>
#include <innodb/log_redo/log_writer_mutex_exit.h>
#include <innodb/log_redo/log_writer_mutex_own.h>
#include <innodb/log_redo/log_write_notifier_mutex_enter.h>
#include <innodb/log_redo/log_write_notifier_mutex_exit.h>
#include <innodb/log_redo/log_write_notifier_mutex_own.h>



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

/** Calculates margin which has to be used in log_free_check() call,
when checking if user thread should wait for more space in redo log.
@return size of the margin to use */
sn_t log_free_check_margin(const log_t &log);

/** Waits until there is free space in log files which includes
concurrency margin required for all threads. You should rather
use log_free_check().
@see @ref sect_redo_log_reclaim_space
@param[in]	log   redo log
@param[in]	sn    sn for which there should be concurrency margin */
void log_free_check_wait(log_t &log, sn_t sn);

/** Updates sn limit values up to which user threads may consider the
reserved space as available both in the log buffer and in the log files.
Both limits - for the start and for the end of reservation, are updated.
Limit for the end is the only one, which truly guarantees that there is
space for the whole reservation. Limit for the start is used to check
free space when being outside mtr (without latches), in which case it
is unknown how much we will need to reserve and write, so current sn
is then compared to the limit. This is called whenever these limits
may change - when write_lsn or last_checkpoint_lsn are advanced,
when the log buffer is resized or margins are changed (e.g. because
of changed concurrency limit).
@param[in,out]	log	redo log */
void log_update_limits(log_t &log);

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

/** Fill redo log header.
@param[out]	buf		filled buffer
@param[in]	start_lsn	log start LSN
@param[in]	creator		creator of the header */
void log_files_header_fill(byte *buf, lsn_t start_lsn, const char *creator);

/** Writes a log file header to the log file space.
@param[in]	log		redo log
@param[in]	nth_file	header for the nth file in the log files
@param[in]	start_lsn	log file data starts at this lsn */
void log_files_header_flush(log_t &log, uint32_t nth_file, lsn_t start_lsn);

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

/** Updates current_file_lsn and current_file_real_offset to correspond
to a given lsn. For this function to work, the values must already be
initialized to correspond to some lsn, for instance, a checkpoint lsn.
@param[in,out]	log	redo log
@param[in]	lsn	log sequence number to set files_start_lsn at */
void log_files_update_offsets(log_t &log, lsn_t lsn);








#endif /* !UNIV_HOTBACKUP */

/** Calculates offset within log files, excluding headers of log files.
@param[in]	log		redo log
@param[in]	offset		real offset (including log file headers)
@return	size offset excluding log file headers (<= offset) */
uint64_t log_files_size_offset(const log_t &log, uint64_t offset);

/** Calculates offset within log files, including headers of log files.
@param[in]	log		redo log
@param[in]	offset		size offset (excluding log file headers)
@return real offset including log file headers (>= offset) */
uint64_t log_files_real_offset(const log_t &log, uint64_t offset);

/** Calculates offset within log files, including headers of log files,
for the provided lsn value.
@param[in]	log	redo log
@param[in]	lsn	log sequence number
@return real offset within the log files */
uint64_t log_files_real_offset_for_lsn(const log_t &log, lsn_t lsn);
#ifndef UNIV_HOTBACKUP

/** Changes size of the log buffer. This is a thread-safe version.
It is used by SET GLOBAL innodb_log_buffer_size = X.
@param[in,out]	log		redo log
@param[in]	new_size	requested new size
@return true iff succeeded in resize */
bool log_buffer_resize(log_t &log, size_t new_size);

/** Changes size of the log buffer. This is a non-thread-safe version
which might be invoked only when there are no concurrent possible writes
to the log buffer. It is used in log_buffer_reserve() when a requested
size to reserve is larger than size of the log buffer.
@param[in,out]	log		redo log
@param[in]	new_size	requested new size
@param[in]	end_lsn		maximum lsn written to log buffer
@return true iff succeeded in resize */
bool log_buffer_resize_low(log_t &log, size_t new_size, lsn_t end_lsn);

/** Resizes the write ahead buffer in the redo log.
@param[in,out]	log		redo log
@param[in]	new_size	new size (in bytes) */
void log_write_ahead_resize(log_t &log, size_t new_size);

/** Increase concurrency_margin used inside log_free_check() calls. */
void log_increase_concurrency_margin(log_t &log);

/** Prints information about important lsn values used in the redo log,
and some statistics about speed of writing and flushing of data.
@param[in]	log	redo log for which print information
@param[out]	file	file where to print */
void log_print(const log_t &log, FILE *file);

/** Refreshes the statistics used to print per-second averages in log_print().
@param[in,out]	log	redo log */
void log_refresh_stats(log_t &log);

/** Creates the first checkpoint ever in the log files. Used during
initialization of new log files. Flushes:
  - header of the first log file (including checkpoint headers),
  - log block with data addressed by the checkpoint lsn.
@param[in,out]	log	redo log
@param[in]	lsn	the first checkpoint lsn */
void log_create_first_checkpoint(log_t &log, lsn_t lsn);

/** Calculates limits for maximum age of checkpoint and maximum age of
the oldest page. Uses current value of srv_thread_concurrency.
@param[in,out]	log	redo log
@retval true if success
@retval false if the redo log is too small to accommodate the number of
OS threads in the database server */
bool log_calc_max_ages(log_t &log);

/** Initializes the log system. Note that the log system is not ready
for user writes after this call is finished. It should be followed by
a call to log_start. Also, log background threads need to be started
manually using log_start_background_threads afterwards.

Hence the proper order of calls looks like this:
        - log_sys_init(),
        - log_start(),
        - log_start_background_threads().

@param[in]	n_files		number of log files
@param[in]	file_size	size of each log file in bytes
@param[in]	space_id	space id of the file space with log files */
bool log_sys_init(uint32_t n_files, uint64_t file_size, space_id_t space_id);

/** Starts the initialized redo log system using a provided
checkpoint_lsn and current lsn.
@param[in,out]	log		redo log
@param[in]	checkpoint_no	checkpoint no (sequential number)
@param[in]	checkpoint_lsn	checkpoint lsn
@param[in]	start_lsn	current lsn to start at */
void log_start(log_t &log, checkpoint_no_t checkpoint_no, lsn_t checkpoint_lsn,
               lsn_t start_lsn);

/** Validates that the log writer thread is active.
Used only to assert, that the state is correct.
@param[in]	log	redo log */
void log_writer_thread_active_validate(const log_t &log);

/** Validates that the log closer thread is active.
Used only to assert, that the state is correct.
@param[in]	log	redo log */
void log_closer_thread_active_validate(const log_t &log);

/** Validates that the log writer, flusher threads are active.
Used only to assert, that the state is correct.
@param[in]	log	redo log */
void log_background_write_threads_active_validate(const log_t &log);

/** Validates that all the log background threads are active.
Used only to assert, that the state is correct.
@param[in]	log	redo log */
void log_background_threads_active_validate(const log_t &log);

/** Validates that all the log background threads are inactive.
Used only to assert, that the state is correct.
@param[in]	log	redo log */
void log_background_threads_inactive_validate(const log_t &log);

/** Starts all the log background threads. This can be called only,
when the threads are inactive. This should never be called concurrently.
This may not be called during read-only mode.
@param[in,out]	log	redo log */
void log_start_background_threads(log_t &log);

/** Stops all the log background threads. This can be called only,
when the threads are active. This should never be called concurrently.
This may not be called in read-only mode. Note that is is impossible
to start log background threads in such case.
@param[in,out]	log	redo log */
void log_stop_background_threads(log_t &log);

/** @return true iff log threads are started */
bool log_threads_active(const log_t &log);

/** Free the log system data structures. Deallocate all the related memory. */
void log_sys_close();

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









/** Lock redo log. Both current lsn and checkpoint lsn will not change
until the redo log is unlocked.
@param[in,out]	log	redo log to lock */
void log_position_lock(log_t &log);

/** Unlock the locked redo log.
@param[in,out]	log	redo log to unlock */
void log_position_unlock(log_t &log);

/** Collect coordinates in the locked redo log.
@param[in]	log		locked redo log
@param[out]	current_lsn	stores current lsn there
@param[out]	checkpoint_lsn	stores checkpoint lsn there */
void log_position_collect_lsn_info(const log_t &log, lsn_t *current_lsn,
                                   lsn_t *checkpoint_lsn);

#else /* !UNIV_HOTBACKUP */

#ifdef UNIV_DEBUG

/** Print a log file header.
@param[in]	block	pointer to the log buffer */
void meb_log_print_file_hdr(byte *block);

#endif /* UNIV_DEBUG */

#endif /* !UNIV_HOTBACKUP */



#endif /* !log0log_h */
