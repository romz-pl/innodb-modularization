/*****************************************************************************

Copyright (c) 1997, 2018, Oracle and/or its affiliates. All Rights Reserved.

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

/** @file include/log0recv.h
 Recovery

 Created 9/20/1997 Heikki Tuuri
 *******************************************************/

#ifndef log0recv_h
#define log0recv_h

#include <innodb/univ/univ.h>

#include <innodb/log_recv/recv_sys_var_init.h>
#include <innodb/log_recv/recv_sys_close.h>
#include <innodb/log_recv/recv_sys_create.h>
#include <innodb/log_recv/recv_calc_lsn_on_data_add.h>
#include <innodb/log_recv/recv_encr_ts_list.h>
#include <innodb/log_recv/recv_n_pool_free_frames.h>
#include <innodb/log_recv/flags.h>
#include <innodb/log_recv/recv_lsn_checks_on.h>
#include <innodb/log_recv/recv_needed_recovery.h>
#include <innodb/log_recv/recv_sys.h>
#include <innodb/log_recv/recv_sys_t.h>
#include <innodb/log_recv/MetadataRecover.h>
#include <innodb/log_recv/recv_dblwr_t.h>
#include <innodb/log_recv/recv_addr_t.h>
#include <innodb/log_recv/recv_addr_state.h>
#include <innodb/log_recv/recv_t.h>
#include <innodb/log_recv/recv_data_t.h>
#include <innodb/log_types/mlog_id_t.h>
#include <innodb/allocator/ut_allocator.h>
#include <innodb/buf_page/buf_flush_t.h>
#include <innodb/memory/mem_heap_t.h>
#include <innodb/log_types/recv_recovery_on.h>
#include <innodb/log_types/log_t.h>

#include <innodb/log_types/recv_recovery_on.h>
#include <innodb/log_write/recv_no_ibuf_operations.h>

#include "dict0types.h"





#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <vector>

class MetadataRecover;
class PersistentTableMetadata;
struct buf_block_t;

#ifdef UNIV_HOTBACKUP

struct recv_addr_t;

/** This is set to FALSE if the backup was originally taken with the
mysqlbackup --include regexp option: then we do not want to create tables in
directories which were not included */
extern bool meb_replay_file_ops;
/** true if the redo log is copied during an online backup */
extern volatile bool is_online_redo_copy;
/** the last redo log flush len as seen by MEB */
extern volatile lsn_t backup_redo_log_flushed_lsn;
/** TRUE when the redo log is being backed up */
extern bool recv_is_making_a_backup;

/** Scans the log segment and n_bytes_scanned is set to the length of valid
log scanned.
@param[in]	buf			buffer containing log data
@param[in]	buf_len			data length in that buffer
@param[in,out]	scanned_lsn		lsn of buffer start, we return scanned
lsn
@param[in,out]	scanned_checkpoint_no	4 lowest bytes of the highest scanned
checkpoint number so far
@param[out]	n_bytes_scanned		how much we were able to scan, smaller
than buf_len if log data ended here */
void meb_scan_log_seg(byte *buf, ulint buf_len, lsn_t *scanned_lsn,
                      ulint *scanned_checkpoint_no, ulint *n_bytes_scanned);

/** Applies the hashed log records to the page, if the page lsn is less than the
lsn of a log record. This can be called when a buffer page has just been
read in, or also for a page already in the buffer pool.
@param[in,out]	block		buffer block */
void recv_recover_page_func(buf_block_t *block);

/** Wrapper for recv_recover_page_func().
Applies the hashed log records to the page, if the page lsn is less than the
lsn of a log record. This can be called when a buffer page has just been
read in, or also for a page already in the buffer pool.
@param jri in: TRUE if just read in (the i/o handler calls this for
a freshly read page)
@param block in,out: the buffer block
*/
#define recv_recover_page(jri, block) recv_recover_page_func(block)

/** Applies log records in the hash table to a backup. */
void meb_apply_log_recs(void);

/** Applies log records in the hash table to a backup using a callback
functions.
@param[in]	function		function for apply
@param[in]	wait_till_finished	function for wait */
void meb_apply_log_recs_via_callback(
    void (*apply_log_record_function)(recv_addr_t *),
    void (*wait_till_done_function)());

/** Applies a log record in the hash table to a backup.
@param[in]	recv_addr	chain of log records
@param[in,out]	block		buffer block to apply the records to */
void meb_apply_log_record(recv_addr_t *recv_addr, buf_block_t *block);

/** Process a file name passed as an input
@param[in]	name		absolute path of tablespace file
@param[in]	space_id	the tablespace ID
@retval		true		if able to process file successfully.
@retval		false		if unable to process the file */
void meb_fil_name_process(const char *name, space_id_t space_id);

/** Scans log from a buffer and stores new log data to the parsing buffer.
Parses and hashes the log records if new data found.  Unless
UNIV_HOTBACKUP is defined, this function will apply log records
automatically when the hash table becomes full.
@param[in]	available_memory	we let the hash table of recs
to grow to this size, at the maximum
@param[in]	buf			buffer containing a log
segment or garbage
@param[in]	len			buffer length
@param[in]	checkpoint_lsn		latest checkpoint LSN
@param[in]	start_lsn		buffer start lsn
@param[in]	contiguous_lsn		it is known that all log
groups contain contiguous log data up to this lsn
@param[out]	group_scanned_lsn	scanning succeeded up to this lsn
@retval	true	if limit_lsn has been reached, or not able to scan any
more in this log group
@retval	false	otherwise */
bool meb_scan_log_recs(ulint available_memory, const byte *buf, ulint len,
                       lsn_t checkpoint_lsn, lsn_t start_lsn,
                       lsn_t *contiguous_lsn, lsn_t *group_scanned_lsn);

bool recv_check_log_header_checksum(const byte *buf);
#else /* UNIV_HOTBACKUP */

/** Applies the hashed log records to the page, if the page lsn is less than the
lsn of a log record. This can be called when a buffer page has just been
read in, or also for a page already in the buffer pool.
@param[in]	just_read_in	true if the IO handler calls this for a freshly
                                read page
@param[in,out]	block		buffer block */
void recv_recover_page_func(bool just_read_in, buf_block_t *block);

/** Wrapper for recv_recover_page_func().
Applies the hashed log records to the page, if the page lsn is less than the
lsn of a log record. This can be called when a buffer page has just been
read in, or also for a page already in the buffer pool.
@param jri in: TRUE if just read in (the i/o handler calls this for
a freshly read page)
@param[in,out]	block	buffer block */
#define recv_recover_page(jri, block) recv_recover_page_func(jri, block)

/** Frees the recovery system. */
void recv_sys_free();


#endif /* UNIV_HOTBACKUP */

#ifdef UNIV_HOTBACKUP
/** Get the number of bytes used by all the heaps
@return number of bytes used */
size_t meb_heap_used();
#endif /* UNIV_HOTBACKUP */


/** Start recovering from a redo log checkpoint.
@see recv_recovery_from_checkpoint_finish
@param[in,out]	log		redo log
@param[in]	flush_lsn	FIL_PAGE_FILE_FLUSH_LSN
                                of first system tablespace page
@return error code or DB_SUCCESS */
dberr_t recv_recovery_from_checkpoint_start(log_t &log, lsn_t flush_lsn)
    MY_ATTRIBUTE((warn_unused_result));

/** Complete the recovery from the latest checkpoint.
@param[in,out]	log		redo log
@param[in]	aborting	true if the server has to abort due to an error
@return recovered persistent metadata or nullptr if aborting*/
MetadataRecover *recv_recovery_from_checkpoint_finish(log_t &log, bool aborting)
    MY_ATTRIBUTE((warn_unused_result));



/** Inits the recovery system for a recovery operation.
@param[in]	max_mem		Available memory in bytes */
void recv_sys_init(ulint max_mem);


/** Empties the hash table of stored log records, applying them to appropriate
pages.
@param[in,out]	log		redo log
@param[in]	allow_ibuf	if true, ibuf operations are allowed during
                                the application; if false, no ibuf operations
                                are allowed, and after the application all
                                file pages are flushed to disk and invalidated
                                in buffer pool: this alternative means that
                                no new log records can be generated during
                                the application; the caller must in this case
                                own the log mutex */
void recv_apply_hashed_log_recs(log_t &log, bool allow_ibuf);

#if defined(UNIV_DEBUG) || defined(UNIV_HOTBACKUP)
/** Return string name of the redo log record type.
@param[in]	type	record log record enum
@return string name of record log record */
const char *get_mlog_string(mlog_id_t type);
#endif /* UNIV_DEBUG || UNIV_HOTBACKUP */




#include "log0recv.ic"

#endif
