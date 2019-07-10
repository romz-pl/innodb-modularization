/***********************************************************************

Copyright (c) 1995, 2018, Oracle and/or its affiliates. All Rights Reserved.
Copyright (c) 2009, Percona Inc.

Portions of this file contain modifications contributed and copyrighted
by Percona Inc.. Those modifications are
gratefully acknowledged and are described briefly in the InnoDB
documentation. The contributions by Percona Inc. are incorporated with
their permission, and subject to the conditions contained in the file
COPYING.Percona.

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

***********************************************************************/

/** @file include/os0file.h
 The interface to the operating system file io

 Created 10/21/1995 Heikki Tuuri
 *******************************************************/

#ifndef os0file_h
#define os0file_h

#include <innodb/univ/univ.h>
#include <innodb/assert/assert.h>
#include <innodb/error/dberr_t.h>

#include <innodb/io/Encryption.h>
#include <innodb/io/os_offset_t.h>
#include <innodb/io/os_file_t.h>
#include <innodb/io/os_file_create_t.h>
#include <innodb/io/IORequest.h>
#include <innodb/io/os_file_size_t.h>
#include <innodb/ioasync/AIO_mode.h>
#include <innodb/io/os_file_type_t.h>
#include <innodb/io/os_file_stat_t.h>
#include <innodb/io/os_file_get_last_error_low.h>
#include <innodb/io/os_file_get_last_error.h>
#include <innodb/io/os_file_handle_error_cond_exit.h>
#include <innodb/io/os_file_handle_error.h>
#include <innodb/io/os_file_handle_error_no_exit.h>
#include <innodb/io/os_file_create_directory.h>
#include <innodb/io/os_file_scan_directory.h>
#include <innodb/io/os_file_is_root.h>
#include <innodb/io/os_file_get_parent_dir.h>
#include <innodb/io/os_file_status_posix.h>
#include <innodb/io/os_file_lock.h>
#include <innodb/io/os_file_create_simple_func.h>
#include <innodb/io/os_file_set_nocache.h>
#include <innodb/io/os_file_set_umask.h>
#include <innodb/io/os_innodb_umask.h>
#include <innodb/io/os_file_rename_func.h>
#include <innodb/io/os_file_close_func.h>
#include <innodb/io/os_file_get_status_posix.h>
#include <innodb/io/os_file_delete_func.h>
#include <innodb/io/os_file_get_size.h>
#include <innodb/io/os_file_create_func.h>
#include <innodb/io/access_type.h>
#include <innodb/io/os_file_create_simple_no_error_handling_func.h>
#include <innodb/io/os_create_block_cache.h>
#include <innodb/io/os_alloc_block.h>
#include <innodb/io/os_free_block.h>
#include <innodb/io/meb_free_block_cache.h>
#include <innodb/io/os_file_compress_page.h>
#include <innodb/io/os_file_fsync_posix.h>
#include <innodb/io/os_file_flush_func.h>
#include <innodb/io/os_file_can_delete.h>
#include <innodb/io/os_file_delete_if_exists_func.h>
#include <innodb/io/Dir_Walker.h>
#include <innodb/io/os_is_o_direct_supported.h>
#include <innodb/io/os_file_punch_hole_posix.h>
#include <innodb/io/os_file_punch_hole.h>
#include <innodb/io/os_is_sparse_file_supported.h>
#include <innodb/io/os_file_original_page_size.h>
#include <innodb/io/os_file_compressed_page_size.h>
#include <innodb/io/os_file_get_status.h>
#include <innodb/io/os_file_create_subdirs_if_needed.h>
#include <innodb/io/os_file_status.h>
#include <innodb/io/os_file_encrypt_page.h>
#include <innodb/io/os_file_encrypt_log.h>
#include <innodb/io/SyncFileIO.h>
#include <innodb/io/os_file_io_complete.h>
#include <innodb/io/os_file_io.h>
#include <innodb/io/os_file_seek.h>
#include <innodb/io/os_file_truncate_posix.h>
#include <innodb/io/os_file_set_eof.h>
#include <innodb/io/os_file_truncate.h>
#include <innodb/io/os_n_pending_writes.h>
#include <innodb/io/os_file_pwrite.h>
#include <innodb/io/os_n_file_writes.h>
#include <innodb/io/os_file_write_page.h>
#include <innodb/io/os_n_file_reads.h>
#include <innodb/io/os_n_pending_reads.h>
#include <innodb/io/os_file_pread.h>
#include <innodb/io/os_file_write_func.h>
#include <innodb/io/os_file_read_no_error_handling_func.h>
#include <innodb/io/OS_FILE_FROM_FD.h>
#include <innodb/io/OS_FILE_CLOSE_FD.h>
#include <innodb/io/OS_FD_FROM_FILE.h>
#include <innodb/io/os_file_close_pfs.h>
#include <innodb/io/os_file_close.h>
#include <innodb/io/os_file_copy_read_write.h>
#include <innodb/io/os_file_copy_func.h>
#include <innodb/io/os_file_read_func.h>
#include <innodb/io/os_fusionio_get_sector_size.h>
#include <innodb/io/os_fsync_threshold.h>
#include <innodb/io/pfs_os_file_create_simple_func.h>
#include <innodb/io/pfs_os_file_create_simple_no_error_handling_func.h>
#include <innodb/io/pfs_os_file_create_func.h>
#include <innodb/io/pfs_os_file_close_func.h>
#include <innodb/io/register_pfs_file_open_begin.h>
#include <innodb/io/register_pfs_file_open_end.h>
#include <innodb/io/register_pfs_file_rename_begin.h>
#include <innodb/io/register_pfs_file_rename_end.h>
#include <innodb/io/register_pfs_file_close_begin.h>
#include <innodb/io/register_pfs_file_close_end.h>
#include <innodb/io/register_pfs_file_io_begin.h>
#include <innodb/io/register_pfs_file_io_end.h>
#include <innodb/io/pfs_os_file_read_func.h>
#include <innodb/io/pfs_os_file_copy_func.h>
#include <innodb/io/pfs_os_file_read_no_error_handling_func.h>
#include <innodb/io/pfs_os_file_read_no_error_handling_int_fd_func.h>
#include <innodb/io/pfs_os_file_write_func.h>
#include <innodb/io/pfs_os_file_write_int_fd_func.h>
#include <innodb/io/pfs_os_file_flush_func.h>
#include <innodb/io/pfs_os_file_rename_func.h>
#include <innodb/io/pfs_os_file_delete_func.h>
#include <innodb/io/pfs_os_file_delete_if_exists_func.h>
#include <innodb/io/os_file_create.h>
#include <innodb/io/os_file_create_simple.h>
#include <innodb/io/os_file_create_simple_no_error_handling.h>
#include <innodb/io/os_file_read_pfs.h>
#include <innodb/io/os_file_copy_pfs.h>
#include <innodb/io/os_file_read_no_error_handling_pfs.h>
#include <innodb/io/os_file_read_no_error_handling_int_fd.h>
#include <innodb/io/os_file_write_pfs.h>
#include <innodb/io/os_file_write_int_fd.h>
#include <innodb/io/os_file_flush_pfs.h>
#include <innodb/io/os_file_rename.h>
#include <innodb/io/os_file_delete.h>
#include <innodb/io/os_file_delete_if_exists.h>
#include <innodb/io/os_file_read.h>
#include <innodb/io/os_file_flush.h>
#include <innodb/io/os_file_write.h>
#include <innodb/io/os_file_copy.h>
#include <innodb/io/os_file_read_no_error_handling.h>
#include <innodb/io/os_file_close_no_error_handling.h>
#include <innodb/io/os_file_read_string.h>



#include "my_dbug.h"
#include "my_io.h"
#include "os/file.h"


#ifndef _WIN32
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#else
#include <Strsafe.h>
#include <locale>
#include <string>
#endif /* !_WIN32 */

#include <functional>
#include <stack>

/** File node of a tablespace or the log data space */
struct fil_node_t;


#ifdef _WIN32

typedef HANDLE os_file_dir_t; /*!< directory stream */

/** We define always WIN_ASYNC_IO, and check at run-time whether
the OS actually supports it: Win 95 does not, NT does. */
#define WIN_ASYNC_IO

/** Use unbuffered I/O */
#define UNIV_NON_BUFFERED_IO

#endif /* _WIN32 */





#ifndef UNIV_HOTBACKUP
/** Create a temporary file. This function is like tmpfile(3), but
the temporary file is created in the given parameter path. If the path
is null then it will create the file in the mysql server configuration
parameter (--tmpdir).
@param[in]	path	location for creating temporary file
@return temporary file handle, or NULL on error */
FILE *os_file_create_tmpfile(const char *path);
#endif /* !UNIV_HOTBACKUP */




#ifdef UNIV_PFS_IO

/* Keys to register InnoDB I/O with performance schema */
extern mysql_pfs_key_t innodb_tablespace_open_file_key;

#define os_file_read_first_page_pfs(type, file, buf, n) \
  pfs_os_file_read_first_page_func(type, file, buf, n, __FILE__, __LINE__)


#else /* UNIV_PFS_IO */

#define os_file_read_first_page_pfs(type, file, buf, n) \
  os_file_read_first_page_func(type, file, buf, n)

#endif /* UNIV_PFS_IO */






#ifdef UNIV_PFS_IO
#define os_file_read_first_page(type, file, buf, n) \
  os_file_read_first_page_pfs(type, file, buf, n)
#else
#define os_file_read_first_page(type, file, buf, n) \
  os_file_read_first_page_pfs(type, file.m_file, buf, n)
#endif

















/** NOTE! Use the corresponding macro os_file_read_first_page(),
not directly this function!
Requests a synchronous read operation of page 0 of IBD file
@param[in]	type		IO request context
@param[in]	file		Open file handle
@param[out]	buf		buffer where to read
@param[in]	n		number of bytes to read
@return DB_SUCCESS if request was successful */
dberr_t os_file_read_first_page_func(IORequest &type, os_file_t file, void *buf,
                                     ulint n)
    MY_ATTRIBUTE((warn_unused_result));






#ifdef UNIV_ENABLE_UNIT_TEST_GET_PARENT_DIR
/* Test the function os_file_get_parent_dir. */
void unit_test_os_file_get_parent_dir();
#endif /* UNIV_ENABLE_UNIT_TEST_GET_PARENT_DIR */

















#ifndef UNIV_HOTBACKUP

/** return any of the tmpdir path */
char *innobase_mysql_tmpdir();
/** Creates a temporary file in the location specified by the parameter
path. If the path is NULL then it will be created on --tmpdir location.
This function is defined in ha_innodb.cc.
@param[in]	path	location for creating temporary file
@return temporary file descriptor, or < 0 on error */
int innobase_mysql_tmpfile(const char *path);
#endif /* !UNIV_HOTBACKUP */













/** Decompress the page data contents. Page type must be FIL_PAGE_COMPRESSED, if
not then the source contents are left unchanged and DB_SUCCESS is returned.
@param[in]	dblwr_recover	true of double write recovery in progress
@param[in,out]	src		Data read from disk, decompressed data will be
                                copied to this page
@param[in,out]	dst		Scratch area to use for decompression
@param[in]	dst_len		Size of the scratch area in bytes
@return DB_SUCCESS or error code */
dberr_t os_file_decompress_page(bool dblwr_recover, byte *src, byte *dst,
                                ulint dst_len)
    MY_ATTRIBUTE((warn_unused_result));



#include "os0file.ic"

#endif /* os0file_h */
