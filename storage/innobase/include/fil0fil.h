/*****************************************************************************

Copyright (c) 1995, 2019, Oracle and/or its affiliates. All Rights Reserved.

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

/** @file include/fil0fil.h
 The low-level file system

 Created 10/25/1995 Heikki Tuuri
 *******************************************************/

#ifndef fil0fil_h
#define fil0fil_h

#include <innodb/univ/univ.h>

#include <innodb/tablespace/fil_reset_encryption.h>
#include <innodb/tablespace/fil_get_compression.h>
#include <innodb/tablespace/fil_set_encryption.h>
#include <innodb/tablespace/fil_validate_skip.h>
#include <innodb/tablespace/fil_encryption_rotate.h>
#include <innodb/tablespace/fil_tablespace_open_for_recovery.h>
#include <innodb/tablespace/fil_open_for_business.h>
#include <innodb/tablespace/fil_space_update_name.h>
#include <innodb/tablespace/fil_rename_tablespace.h>
#include <innodb/tablespace/fil_rename_tablespace_by_id.h>
#include <innodb/tablespace/fil_free_scanned_files.h>
#include <innodb/tablespace/fil_get_dirs.h>
#include <innodb/tablespace/fil_check_path.h>
#include <innodb/tablespace/fil_tablespace_lookup_for_recovery.h>
#include <innodb/tablespace/fil_space_get_sys_space.h>
#include <innodb/tablespace/fil_fusionio_enable_atomic_write.h>
#include <innodb/tablespace/fil_space_open_if_needed.h>
#include <innodb/tablespace/fil_path_to_space_name.h>
#include <innodb/tablespace/fil_space_read_name_and_filepath.h>
#include <innodb/tablespace/PageCallback.h>
#include <innodb/tablespace/fil_delete_file.h>
#include <innodb/tablespace/fil_addr_is_null.h>
#include <innodb/tablespace/fil_validate.h>
#include <innodb/tablespace/fil_flush_file_spaces.h>
#include <innodb/tablespace/fil_flush_file_redo.h>
#include <innodb/tablespace/fil_flush.h>
#include <innodb/tablespace/fil_redo_io.h>
#include <innodb/tablespace/fil_space_get_n_reserved_extents.h>
#include <innodb/tablespace/fil_space_release_free_extents.h>
#include <innodb/tablespace/fil_space_reserve_free_extents.h>
#include <innodb/page/page_type_t.h>
#include <innodb/io/Fil_path.h>
#include <innodb/io/ib_file_suffix.h>
#include <innodb/io/Encryption.h>
#include <innodb/io/Compression.h>
#include <innodb/io/pfs_os_file_t.h>
#include <innodb/io/os_offset_t.h>
#include <innodb/tablespace/fil_type_t.h>
#include <innodb/tablespace/Fil_state.h>
#include <innodb/tablespace/space_id_t.h>
#include <innodb/tablespace/fil_node_t.h>
#include <innodb/tablespace/encryption_op_type.h>
#include <innodb/tablespace/fil_space_t.h>
#include <innodb/tablespace/fil_faddr_t.h>
#include <innodb/tablespace/fil_addr_t.h>
#include <innodb/buffer/buf_frame_t.h>
#include <innodb/tablespace/buf_remove_t.h>
#include <innodb/tablespace/fil_no_punch_hole.h>
#include <innodb/tablespace/consts.h>
#include <innodb/tablespace/Space_ids.h>
#include <innodb/tablespace/Filenames.h>
#include <innodb/io/MySQL_datadir_path.h>
#include <innodb/tablespace/fil_addr_null.h>
#include <innodb/tablespace/fil_n_log_flushes.h>
#include <innodb/tablespace/fil_n_pending_log_flushes.h>
#include <innodb/tablespace/fil_n_pending_tablespace_flushes.h>
#include <innodb/tablespace/fil_space_get.h>
#include <innodb/tablespace/fil_space_get_latch.h>
#include <innodb/tablespace/fil_space_get_type.h>
#include <innodb/tablespace/fil_space_set_imported.h>
#include <innodb/tablespace/fil_node_create.h>
#include <innodb/tablespace/fil_assign_new_space_id.h>
#include <innodb/tablespace/fil_space_get_first_path.h>
#include <innodb/tablespace/fil_space_get_size.h>
#include <innodb/tablespace/fil_space_get_flags.h>
#include <innodb/tablespace/fil_space_set_flags.h>
#include <innodb/tablespace/fil_space_open.h>
#include <innodb/tablespace/fil_space_close.h>
#include <innodb/tablespace/fil_space_get_page_size.h>
#include <innodb/tablespace/fil_init.h>
#include <innodb/tablespace/fil_close.h>

#include "dict0types.h"
#include <innodb/page/type.h>
#include "log0recv.h"
#include <innodb/machine/data.h>
#include <innodb/page/page_size_t.h>
#ifndef UNIV_HOTBACKUP
#include "ibuf0types.h"
#endif /* !UNIV_HOTBACKUP */

#include "sql/dd/object_id.h"

#include <list>
#include <vector>
#include <functional>

struct buf_block_t;

/** This tablespace name is used internally during file discovery to open a
general tablespace before the data dictionary is recovered and available. */
static constexpr char general_space_name[] = "innodb_general";

/** This tablespace name is used as the prefix for implicit undo tablespaces
and during file discovery to open an undo tablespace before the DD is
recovered and available. */
static constexpr char undo_space_name[] = "innodb_undo";

extern volatile bool recv_recovery_on;

#ifdef UNIV_HOTBACKUP
#include <unordered_set>
using Dir_set = std::unordered_set<std::string>;
extern Dir_set rem_gen_ts_dirs;
extern bool replay_in_datadir;
#endif /* UNIV_HOTBACKUP */

// Forward declaration
struct trx_t;
class page_id_t;




#ifdef _WIN32
/* Initialization of m_abs_path() produces warning C4351:
"new behavior: elements of array '...' will be default initialized."
See https://msdn.microsoft.com/en-us/library/1ywe7hcy.aspx */
#pragma warning(disable : 4351)
#endif /* _WIN32 */

























#include <innodb/tablespace/fil_open_log_and_system_tablespace_files.h>
#include <innodb/tablespace/fil_close_all_files.h>
#include <innodb/tablespace/fil_close_log_files.h>
#include <innodb/tablespace/fil_space_acquire_low.h>
#include <innodb/tablespace/fil_space_acquire_silent.h>
#include <innodb/tablespace/fil_space_release.h>
#include <innodb/tablespace/fil_system_open_fetch.h>
#include <innodb/tablespace/fil_truncate_tablespace.h>
#include <innodb/tablespace/fil_delete_tablespace.h>
#include <innodb/tablespace/fil_space_acquire.h>
#include <innodb/tablespace/fil_set_max_space_id_if_bigger.h>
#include <innodb/tablespace/fil_space_create.h>
#include <innodb/tablespace/fil_write_flushed_lsn.h>
#include <innodb/tablespace/fil_write_flushed_lsn.h>
#include <innodb/tablespace/fil_io.h>
#include <innodb/tablespace/fil_replace_tablespace.h>
#include <innodb/tablespace/fil_read.h>
#include <innodb/tablespace/fil_write.h>
#include <innodb/tablespace/fil_space_free.h>
#include <innodb/tablespace/fil_rename_tablespace_check.h>
#include <innodb/tablespace/fil_space_exists_in_mem.h>
#include <innodb/tablespace/fil_space_get_id_by_name.h>
#include <innodb/tablespace/fil_space_extend.h>

#ifndef UNIV_HOTBACKUP


#else /* !UNIV_HOTBACKUP */
/** Extends all tablespaces to the size stored in the space header. During the
mysqlbackup --apply-log phase we extended the spaces on-demand so that log
records could be applied, but that may have left spaces still too small
compared to the size stored in the space header. */
void meb_extend_tablespaces_to_stored_len();

/** Process a file name passed as an input
@param[in]	name		absolute path of tablespace file
@param[in]	space_id	the tablespace ID */
void meb_fil_name_process(const char *name, space_id_t space_id);

#endif /* !UNIV_HOTBACKUP */












/** Closes a single-table tablespace. The tablespace must be cached in the
memory cache. Free all pages used by the tablespace.
@param[in,out]	trx		Transaction covering the close
@param[in]	space_id	Tablespace ID
@return DB_SUCCESS or error */
dberr_t fil_close_tablespace(trx_t *trx, space_id_t space_id)
    MY_ATTRIBUTE((warn_unused_result));

/** Discards a single-table tablespace. The tablespace must be cached in the
memory cache. Discarding is like deleting a tablespace, but

 1. We do not drop the table from the data dictionary;

 2. We remove all insert buffer entries for the tablespace immediately;
    in DROP TABLE they are only removed gradually in the background;

 3. When the user does IMPORT TABLESPACE, the tablespace will have the
    same id as it originally had.

 4. Free all the pages in use by the tablespace if rename=true.
@param[in]	space_id	Tablespace ID
@return DB_SUCCESS or error */
dberr_t fil_discard_tablespace(space_id_t space_id)
    MY_ATTRIBUTE((warn_unused_result));





/** Create a tablespace file.
@param[in]	space_id	Tablespace ID
@param[in]	name		Tablespace name in dbname/tablename format.
                                For general tablespaces, the 'dbname/' part
                                may be missing.
@param[in]	path		Path and filename of the datafile to create.
@param[in]	flags		Tablespace flags
@param[in]	size		Initial size of the tablespace file in pages,
                                must be >= FIL_IBD_FILE_INITIAL_SIZE
@return DB_SUCCESS or error code */
dberr_t fil_ibd_create(space_id_t space_id, const char *name, const char *path,
                       uint32_t flags, page_no_t size)
    MY_ATTRIBUTE((warn_unused_result));

/** Create a session temporary tablespace (IBT) file.
@param[in]	space_id	Tablespace ID
@param[in]	name		Tablespace name
@param[in]	path		Path and filename of the datafile to create.
@param[in]	flags		Tablespace flags
@param[in]	size		Initial size of the tablespace file in pages,
                                must be >= FIL_IBT_FILE_INITIAL_SIZE
@return DB_SUCCESS or error code */
dberr_t fil_ibt_create(space_id_t space_id, const char *name, const char *path,
                       uint32_t flags, page_no_t size)
    MY_ATTRIBUTE((warn_unused_result));



/** Open a single-table tablespace and optionally check the space id is
right in it. If not successful, print an error message to the error log. This
function is used to open a tablespace when we start up mysqld, and also in
IMPORT TABLESPACE.
NOTE that we assume this operation is used either at the database startup
or under the protection of the dictionary mutex, so that two users cannot
race here.

The fil_node_t::handle will not be left open.

@param[in]	validate	whether we should validate the tablespace
                                (read the first page of the file and
                                check that the space id in it matches id)
@param[in]	purpose		FIL_TYPE_TABLESPACE or FIL_TYPE_TEMPORARY
@param[in]	space_id	Tablespace ID
@param[in]	flags		tablespace flags
@param[in]	space_name	tablespace name of the datafile
                                If file-per-table, it is the table name in the
                                databasename/tablename format
@param[in]	table_name	table name in case need to build filename
from it
@param[in]	path_in		expected filepath, usually read from dictionary
@param[in]	strict		whether to report error when open ibd failed
@param[in]	old_space	whether it is a 5.7 tablespace opening
                                by upgrade
@return DB_SUCCESS or error code */
dberr_t fil_ibd_open(bool validate, fil_type_t purpose, space_id_t space_id,
                     uint32_t flags, const char *space_name,
                     const char *table_name, const char *path_in, bool strict,
                     bool old_space) MY_ATTRIBUTE((warn_unused_result));



/** Extends all tablespaces to the size stored in the space header. During the
mysqlbackup --apply-log phase we extended the spaces on-demand so that log
records could be appllied, but that may have left spaces still too small
compared to the size stored in the space header. */
void fil_extend_tablespaces_to_stored_len();













/** Waits for an aio operation to complete. This function is used to write the
handler for completed requests. The aio array of pending requests is divided
into segments (see os0file.cc for more info). The thread specifies which
segment it wants to wait for.
@param[in]	segment		The number of the segment in the AIO array
                                to wait for */
void fil_aio_wait(ulint segment);














/** Reset the page type.
Data files created before MySQL 5.1 may contain garbage in FIL_PAGE_TYPE.
In MySQL 3.23.53, only undo log pages and index pages were tagged.
Any other pages were written with uninitialized bytes in FIL_PAGE_TYPE.
@param[in]	page_id		page number
@param[in,out]	page		page with invalid FIL_PAGE_TYPE
@param[in]	type		expected page type
@param[in,out]	mtr		mini-transaction */
void fil_page_reset_type(const page_id_t &page_id, byte *page, ulint type,
                         mtr_t *mtr);




/** Check (and if needed, reset) the page type.
Data files created before MySQL 5.1 may contain
garbage in the FIL_PAGE_TYPE field.
In MySQL 3.23.53, only undo log pages and index pages were tagged.
Any other pages were written with uninitialized bytes in FIL_PAGE_TYPE.
@param[in]	page_id		page number
@param[in,out]	page		page with possibly invalid FIL_PAGE_TYPE
@param[in]	type		expected page type
@param[in,out]	mtr		mini-transaction */
inline void fil_page_check_type(const page_id_t &page_id, byte *page,
                                ulint type, mtr_t *mtr) {
  ulint page_type = fil_page_get_type(page);

  if (page_type != type) {
    fil_page_reset_type(page_id, page, type, mtr);
  }
}

/** Check (and if needed, reset) the page type.
Data files created before MySQL 5.1 may contain
garbage in the FIL_PAGE_TYPE field.
In MySQL 3.23.53, only undo log pages and index pages were tagged.
Any other pages were written with uninitialized bytes in FIL_PAGE_TYPE.
@param[in,out]	block		block with possibly invalid FIL_PAGE_TYPE
@param[in]	type		expected page type
@param[in,out]	mtr		mini-transaction */
#define fil_block_check_type(block, type, mtr) \
  fil_page_check_type(block->page.id, block->frame, type, mtr)

#ifdef UNIV_DEBUG
/** Increase redo skipped of a tablespace.
@param[in]	space_id	Tablespace ID */
void fil_space_inc_redo_skipped_count(space_id_t space_id);

/** Decrease redo skipped of a tablespace.
@param[in]	space_id	Tablespace ID */
void fil_space_dec_redo_skipped_count(space_id_t space_id);

/** Check whether a single-table tablespace is redo skipped.
@param[in]	space_id	Tablespace ID
@return true if redo skipped */
bool fil_space_is_redo_skipped(space_id_t space_id)
    MY_ATTRIBUTE((warn_unused_result));
#endif /* UNIV_DEBUG */




/** Iterate over all the pages in the tablespace.
@param table the table definiton in the server
@param n_io_buffers number of blocks to read and write together
@param callback functor that will do the page updates
@return DB_SUCCESS or error code */
dberr_t fil_tablespace_iterate(dict_table_t *table, ulint n_io_buffers,
                               PageCallback &callback)
    MY_ATTRIBUTE((warn_unused_result));







/** Check if swapping two .ibd files can be done without failure
@param[in]	old_table	old table
@param[in]	new_table	new table
@param[in]	tmp_name	temporary table name
@return innodb error code */
dberr_t fil_rename_precheck(const dict_table_t *old_table,
                            const dict_table_t *new_table, const char *tmp_name)
    MY_ATTRIBUTE((warn_unused_result));

/** Set the compression type for the tablespace of a table
@param[in]	table		Table that should be compressesed
@param[in]	algorithm	Text representation of the algorithm
@return DB_SUCCESS or error code */
dberr_t fil_set_compression(dict_table_t *table, const char *algorithm)
    MY_ATTRIBUTE((warn_unused_result));



/** Set encryption.
@param[in,out]	req_type	IO request
@param[in]	page_id		Page address for IO
@param[in,out]	space		Tablespace instance */
void fil_io_set_encryption(IORequest &req_type, const page_id_t &page_id,
                           fil_space_t *space);










#ifdef UNIV_ENABLE_UNIT_TEST_MAKE_FILEPATH
void test_make_filepath();
#endif /* UNIV_ENABLE_UNIT_TEST_MAKE_FILEPATH */



/** Redo a tablespace create
@param[in]	ptr		redo log record
@param[in]	end		end of the redo log buffer
@param[in]	page_id		Tablespace Id and first page in file
@param[in]	parsed_bytes	Number of bytes parsed so far
@param[in]	parse_only	Don't apply the log if true
@return pointer to next redo log record
@retval nullptr if this log record was truncated */
byte *fil_tablespace_redo_create(byte *ptr, const byte *end,
                                 const page_id_t &page_id, ulint parsed_bytes,
                                 bool parse_only)
    MY_ATTRIBUTE((warn_unused_result));

/** Redo a tablespace drop
@param[in]	ptr		redo log record
@param[in]	end		end of the redo log buffer
@param[in]	page_id		Tablespace Id and first page in file
@param[in]	parsed_bytes	Number of bytes parsed so far
@param[in]	parse_only	Don't apply the log if true
@return pointer to next redo log record
@retval nullptr if this log record was truncated */
byte *fil_tablespace_redo_delete(byte *ptr, const byte *end,
                                 const page_id_t &page_id, ulint parsed_bytes,
                                 bool parse_only)
    MY_ATTRIBUTE((warn_unused_result));

/** Redo a tablespace rename
@param[in]	ptr		redo log record
@param[in]	end		end of the redo log buffer
@param[in]	page_id		Tablespace Id and first page in file
@param[in]	parsed_bytes	Number of bytes parsed so far
@param[in]	parse_only	Don't apply the log if true
@return pointer to next redo log record
@retval nullptr if this log record was truncated */
byte *fil_tablespace_redo_rename(byte *ptr, const byte *end,
                                 const page_id_t &page_id, ulint parsed_bytes,
                                 bool parse_only)
    MY_ATTRIBUTE((warn_unused_result));

/** Parse and process an encryption redo record.
@param[in]	ptr		redo log record
@param[in]	end		end of the redo log buffer
@param[in]	space_id	the tablespace ID
@return log record end, nullptr if not a complete record */
byte *fil_tablespace_redo_encryption(byte *ptr, const byte *end,
                                     space_id_t space_id)
    MY_ATTRIBUTE((warn_unused_result));




/** Lookup the tablespace ID and return the path to the file. The filename
is ignored when testing for equality. Only the path up to the file name is
considered for matching: e.g. ./test/a.ibd == ./test/b.ibd.
@param[in]	dd_object_id	Server DD tablespace ID
@param[in]	space_id	Tablespace ID to lookup
@param[in]	space_name	Tablespace name
@param[in]	old_path	Path in the data dictionary
@param[out]	new_path	New path if scanned path not equal to path
@return status of the match. */
Fil_state fil_tablespace_path_equals(dd::Object_id dd_object_id,
                                     space_id_t space_id,
                                     const char *space_name,
                                     std::string old_path,
                                     std::string *new_path)
    MY_ATTRIBUTE((warn_unused_result));







/** Callback to check tablespace size with space header size and extend
Caller must own the Fil_shard mutex that the file belongs to.
@param[in]	file	file node
@return	error code */
dberr_t fil_check_extend_space(fil_node_t *file)
    MY_ATTRIBUTE((warn_unused_result));

/** Replay a file rename operation for ddl replay.
@param[in]	page_id		Space ID and first page number in the file
@param[in]	old_name	old file name
@param[in]	new_name	new file name
@return	whether the operation was successfully applied
(the name did not exist, or new_name did not exist and
name was successfully renamed to new_name)  */
bool fil_op_replay_rename_for_ddl(const page_id_t &page_id,
                                  const char *old_name, const char *new_name);





#endif /* fil0fil_h */
