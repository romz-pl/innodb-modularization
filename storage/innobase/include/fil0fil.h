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
#include <innodb/tablespace/MySQL_datadir_path.h>
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


#ifndef UNIV_HOTBACKUP
/** Write the flushed LSN to the page header of the first page in the
system tablespace.
@param[in]	lsn		Flushed LSN
@return DB_SUCCESS or error number */
dberr_t fil_write_flushed_lsn(lsn_t lsn) MY_ATTRIBUTE((warn_unused_result));

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










/** Truncate the tablespace to needed size with a new space_id.
@param[in]  old_space_id   Tablespace ID to truncate
@param[in]  new_space_id   Tablespace ID to for the new file
@param[in]  size_in_pages  Truncate size.
@return true if truncate was successful. */
bool fil_replace_tablespace(space_id_t old_space_id, space_id_t new_space_id,
                            page_no_t size_in_pages)
    MY_ATTRIBUTE((warn_unused_result));

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

/** Test if a tablespace file can be renamed to a new filepath by checking
if that the old filepath exists and the new filepath does not exist.
@param[in]	space_id	Tablespace ID
@param[in]	old_path	Old filepath
@param[in]	new_path	New filepath
@param[in]	is_discarded	Whether the tablespace is discarded
@return innodb error code */
dberr_t fil_rename_tablespace_check(space_id_t space_id, const char *old_path,
                                    const char *new_path, bool is_discarded)
    MY_ATTRIBUTE((warn_unused_result));

/** Rename a single-table tablespace.
The tablespace must exist in the memory cache.
@param[in]	space_id	Tablespace ID
@param[in]	old_path	Old file name
@param[in]	new_name	New tablespace name in the schema/name format
@param[in]	new_path_in	New file name, or nullptr if it is located in
                                The normal data directory
@return InnoDB error code */
dberr_t fil_rename_tablespace(space_id_t space_id, const char *old_path,
                              const char *new_name, const char *new_path_in)
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

/** Returns true if a matching tablespace exists in the InnoDB tablespace
memory cache.
@param[in]	space_id	Tablespace ID
@param[in]	name		Tablespace name used in
                                fil_space_create().
@param[in]	print_err	detailed error information to the
                                error log if a matching tablespace is
                                not found from memory.
@param[in]	adjust_space	Whether to adjust spaceid on mismatch
@param[in]	heap		Heap memory
@param[in]	table_id	table id
@return true if a matching tablespace exists in the memory cache */
bool fil_space_exists_in_mem(space_id_t space_id, const char *name,
                             bool print_err, bool adjust_space,
                             mem_heap_t *heap, table_id_t table_id)
    MY_ATTRIBUTE((warn_unused_result));

/** Extends all tablespaces to the size stored in the space header. During the
mysqlbackup --apply-log phase we extended the spaces on-demand so that log
records could be appllied, but that may have left spaces still too small
compared to the size stored in the space header. */
void fil_extend_tablespaces_to_stored_len();

/** Try to extend a tablespace if it is smaller than the specified size.
@param[in,out]	space		Tablespace ID
@param[in]	size		desired size in pages
@return whether the tablespace is at least as big as requested */
bool fil_space_extend(fil_space_t *space, page_no_t size)
    MY_ATTRIBUTE((warn_unused_result));

/** Tries to reserve free extents in a file space.
@param[in]	space_id	Tablespace ID
@param[in]	n_free_now	Number of free extents now
@param[in]	n_to_reserve	How many one wants to reserve
@return true if succeed */
bool fil_space_reserve_free_extents(space_id_t space_id, ulint n_free_now,
                                    ulint n_to_reserve)
    MY_ATTRIBUTE((warn_unused_result));

/** Releases free extents in a file space.
@param[in]	space_id	Tablespace ID
@param[in]	n_reserved	How many were reserved */
void fil_space_release_free_extents(space_id_t space_id, ulint n_reserved);

/** Gets the number of reserved extents. If the database is silent, this
number should be zero.
@param[in]	space_id	Tablespace ID
@return the number of reserved extents */
ulint fil_space_get_n_reserved_extents(space_id_t space_id)
    MY_ATTRIBUTE((warn_unused_result));

/** Read or write redo log data (synchronous buffered IO).
@param[in]	type		IO context
@param[in]	page_id		where to read or write
@param[in]	page_size	page size
@param[in]	byte_offset	remainder of offset in bytes
@param[in]	len		this must not cross a file boundary;
@param[in,out]	buf		buffer where to store read data or from where
                                to write
@retval DB_SUCCESS if all OK */
dberr_t fil_redo_io(const IORequest &type, const page_id_t &page_id,
                    const page_size_t &page_size, ulint byte_offset, ulint len,
                    void *buf) MY_ATTRIBUTE((warn_unused_result));

/** Read or write data.
@param[in]	type		IO context
@param[in]	sync		If true then do synchronous IO
@param[in]	page_id		page id
@param[in]	page_size	page size
@param[in]	byte_offset	remainder of offset in bytes; in aio this
                                must be divisible by the OS block size
@param[in]	len		how many bytes to read or write; this must
                                not cross a file boundary; in aio this must
                                be a block size multiple
@param[in,out]	buf		buffer where to store read data or from where
                                to write; in aio this must be appropriately
                                aligned
@param[in]	message		message for aio handler if !sync, else ignored
@return error code
@retval DB_SUCCESS on success
@retval DB_TABLESPACE_DELETED if the tablespace does not exist */
dberr_t fil_io(const IORequest &type, bool sync, const page_id_t &page_id,
               const page_size_t &page_size, ulint byte_offset, ulint len,
               void *buf, void *message) MY_ATTRIBUTE((warn_unused_result));

/** Waits for an aio operation to complete. This function is used to write the
handler for completed requests. The aio array of pending requests is divided
into segments (see os0file.cc for more info). The thread specifies which
segment it wants to wait for.
@param[in]	segment		The number of the segment in the AIO array
                                to wait for */
void fil_aio_wait(ulint segment);

/** Flushes to disk possible writes cached by the OS. If the space does
not exist or is being dropped, does not do anything.
@param[in]	space_id	Tablespace ID (this can be a group of log files
                                or a tablespace of the database) */
void fil_flush(space_id_t space_id);

/** Flush to disk the writes in file spaces of the given type
possibly cached by the OS. */
void fil_flush_file_redo();

/** Flush to disk the writes in file spaces of the given type
possibly cached by the OS.
@param[in]	purpose		FIL_TYPE_TABLESPACE or FIL_TYPE_LOG, can
                                be ORred. */
void fil_flush_file_spaces(uint8_t purpose);

#ifdef UNIV_DEBUG
/** Checks the consistency of the tablespace cache.
@return true if ok */
bool fil_validate();
#endif /* UNIV_DEBUG */

/** Returns true if file address is undefined.
@param[in]	addr		File address to check
@return true if undefined */
bool fil_addr_is_null(const fil_addr_t &addr)
    MY_ATTRIBUTE((warn_unused_result));




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

/** Delete the tablespace file and any related files like .cfg.
This should not be called for temporary tables.
@param[in]	path		File path of the tablespace
@return true on success */
bool fil_delete_file(const char *path) MY_ATTRIBUTE((warn_unused_result));

/** Callback functor. */
struct PageCallback {
  /** Default constructor */
  PageCallback() : m_page_size(0, 0, false), m_filepath() UNIV_NOTHROW {}

  virtual ~PageCallback() UNIV_NOTHROW {}

  /** Called for page 0 in the tablespace file at the start.
  @param file_size size of the file in bytes
  @param block contents of the first page in the tablespace file
  @retval DB_SUCCESS or error code. */
  virtual dberr_t init(os_offset_t file_size, const buf_block_t *block)
      MY_ATTRIBUTE((warn_unused_result)) UNIV_NOTHROW = 0;

  /** Called for every page in the tablespace. If the page was not
  updated then its state must be set to BUF_PAGE_NOT_USED. For
  compressed tables the page descriptor memory will be at offset:
  block->frame + UNIV_PAGE_SIZE;
  @param offset physical offset within the file
  @param block block read from file, note it is not from the buffer pool
  @retval DB_SUCCESS or error code. */
  virtual dberr_t operator()(os_offset_t offset, buf_block_t *block)
      MY_ATTRIBUTE((warn_unused_result)) UNIV_NOTHROW = 0;

  /** Set the name of the physical file and the file handle that is used
  to open it for the file that is being iterated over.
  @param filename then physical name of the tablespace file.
  @param file OS file handle */
  void set_file(const char *filename, pfs_os_file_t file) UNIV_NOTHROW {
    m_file = file;
    m_filepath = filename;
  }

  /** @return the space id of the tablespace */
  virtual space_id_t get_space_id() const
      MY_ATTRIBUTE((warn_unused_result)) UNIV_NOTHROW = 0;

  /**
  @retval the space flags of the tablespace being iterated over */
  virtual ulint get_space_flags() const
      MY_ATTRIBUTE((warn_unused_result)) UNIV_NOTHROW = 0;

  /** Set the tablespace table size.
  @param[in] page a page belonging to the tablespace */
  void set_page_size(const buf_frame_t *page) UNIV_NOTHROW;

  /** The compressed page size
  @return the compressed page size */
  const page_size_t &get_page_size() const MY_ATTRIBUTE((warn_unused_result)) {
    return (m_page_size);
  }

  /** The tablespace page size. */
  page_size_t m_page_size;

  /** File handle to the tablespace */
  pfs_os_file_t m_file;

  /** Physical file path. */
  const char *m_filepath;

  // Disable copying
  PageCallback(PageCallback &&) = delete;
  PageCallback(const PageCallback &) = delete;
  PageCallback &operator=(const PageCallback &) = delete;
};

/** Iterate over all the pages in the tablespace.
@param table the table definiton in the server
@param n_io_buffers number of blocks to read and write together
@param callback functor that will do the page updates
@return DB_SUCCESS or error code */
dberr_t fil_tablespace_iterate(dict_table_t *table, ulint n_io_buffers,
                               PageCallback &callback)
    MY_ATTRIBUTE((warn_unused_result));

/** Looks for a pre-existing fil_space_t with the given tablespace ID
and, if found, returns the name and filepath in newly allocated buffers that
the caller must free.
@param[in] space_id The tablespace ID to search for.
@param[out] name Name of the tablespace found.
@param[out] filepath The filepath of the first datafile for thtablespace found.
@return true if tablespace is found, false if not. */
bool fil_space_read_name_and_filepath(space_id_t space_id, char **name,
                                      char **filepath)
    MY_ATTRIBUTE((warn_unused_result));

/** Convert a file name to a tablespace name.
@param[in]	filename	directory/databasename/tablename.ibd
@return database/tablename string, to be freed with ut_free() */
char *fil_path_to_space_name(const char *filename)
    MY_ATTRIBUTE((warn_unused_result));

/** Returns the space ID based on the tablespace name.
The tablespace must be found in the tablespace memory cache.
This call is made from external to this module, so the mutex is not owned.
@param[in]	name		Tablespace name
@return space ID if tablespace found, SPACE_UNKNOWN if space not. */
space_id_t fil_space_get_id_by_name(const char *name)
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

/** Get the compression type for the tablespace
@param[in]	space_id	Space ID to check
@return the compression algorithm */
Compression::Type fil_get_compression(space_id_t space_id)
    MY_ATTRIBUTE((warn_unused_result));

/** Set encryption.
@param[in,out]	req_type	IO request
@param[in]	page_id		Page address for IO
@param[in,out]	space		Tablespace instance */
void fil_io_set_encryption(IORequest &req_type, const page_id_t &page_id,
                           fil_space_t *space);

/** Set the encryption type for the tablespace
@param[in] space_id		Space ID of tablespace for which to set
@param[in] algorithm		Encryption algorithm
@param[in] key			Encryption key
@param[in] iv			Encryption iv
@return DB_SUCCESS or error code */
dberr_t fil_set_encryption(space_id_t space_id, Encryption::Type algorithm,
                           byte *key, byte *iv)
    MY_ATTRIBUTE((warn_unused_result));

/** Reset the encryption type for the tablespace
@param[in] space_id		Space ID of tablespace for which to set
@return DB_SUCCESS or error code */
dberr_t fil_reset_encryption(space_id_t space_id)
    MY_ATTRIBUTE((warn_unused_result));
/** @return true if the re-encrypt success */
bool fil_encryption_rotate() MY_ATTRIBUTE((warn_unused_result));

/** During crash recovery, open a tablespace if it had not been opened
yet, to get valid size and flags.
@param[in,out]	space		Tablespace instance */
inline void fil_space_open_if_needed(fil_space_t *space) {
  if (space->size == 0) {
    /* Initially, size and flags will be set to 0,
    until the files are opened for the first time.
    fil_space_get_size() will open the file
    and adjust the size and flags. */
    page_no_t size = fil_space_get_size(space->id);

    ut_a(size == space->size);
  }
}

#if !defined(NO_FALLOCATE) && defined(UNIV_LINUX)
/**
Try and enable FusionIO atomic writes.
@param[in] file		OS file handle
@return true if successful */
bool fil_fusionio_enable_atomic_write(pfs_os_file_t file)
    MY_ATTRIBUTE((warn_unused_result));
#endif /* !NO_FALLOCATE && UNIV_LINUX */


#ifdef UNIV_ENABLE_UNIT_TEST_MAKE_FILEPATH
void test_make_filepath();
#endif /* UNIV_ENABLE_UNIT_TEST_MAKE_FILEPATH */

/** @return the system tablespace instance */
#define fil_space_get_sys_space() (fil_space_t::s_sys_space)

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

/** Read the tablespace id to path mapping from the file
@param[in]	recovery	true if called from crash recovery */
void fil_tablespace_open_init_for_recovery(bool recovery);

/** Lookup the space ID.
@param[in]	space_id	Tablespace ID to lookup
@return true if space ID is known and open */
bool fil_tablespace_lookup_for_recovery(space_id_t space_id)
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

/** This function should be called after recovery has completed.
Check for tablespace files for which we did not see any MLOG_FILE_DELETE
or MLOG_FILE_RENAME record. These could not be recovered
@return true if there were some filenames missing for which we had to
ignore redo log records during the apply phase */
bool fil_check_missing_tablespaces() MY_ATTRIBUTE((warn_unused_result));

/** Discover tablespaces by reading the header from .ibd files.
@param[in]	directories	Directories to scan
@return DB_SUCCESS if all goes well */
dberr_t fil_scan_for_tablespaces(const std::string &directories);

/** Open the tabelspace and also get the tablespace filenames, space_id must
already be known.
@param[in]	space_id	Tablespace ID to lookup
@return true if open was successful */
bool fil_tablespace_open_for_recovery(space_id_t space_id)
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

/** Free the Tablespace_files instance.
@param[in]	read_only_mode	true if InnoDB is started in read only mode.
@return DB_SUCCESS if all OK */
dberr_t fil_open_for_business(bool read_only_mode)
    MY_ATTRIBUTE((warn_unused_result));

/** Check if a path is known to InnoDB.
@param[in]	path		Path to check
@return true if path is known to InnoDB */
bool fil_check_path(const std::string &path) MY_ATTRIBUTE((warn_unused_result));

/** Get the list of directories that datafiles can reside in.
@return the list of directories 'dir1;dir2;....;dirN' */
std::string fil_get_dirs() MY_ATTRIBUTE((warn_unused_result));

/** Rename a tablespace.  Use the space_id to find the shard.
@param[in]	space_id	tablespace ID
@param[in]	old_name	old tablespace name
@param[in]	new_name	new tablespace name
@return DB_SUCCESS on success */
dberr_t fil_rename_tablespace_by_id(space_id_t space_id, const char *old_name,
                                    const char *new_name)
    MY_ATTRIBUTE((warn_unused_result));

/** Free the data structures required for recovery. */
void fil_free_scanned_files();

/** Update the tablespace name. Incase, the new name
and old name are same, no update done.
@param[in,out]	space		tablespace object on which name
                                will be updated
@param[in]	name		new name for tablespace */
void fil_space_update_name(fil_space_t *space, const char *name);

#endif /* fil0fil_h */
