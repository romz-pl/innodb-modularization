#include <innodb/io/os_is_sparse_file_supported.h>

#include <innodb/io/os_file_punch_hole.h>

/** Check if the file system supports sparse files.

Warning: On POSIX systems we try and punch a hole from offset 0 to
the system configured page size. This should only be called on an empty
file.

Note: On Windows we use the name and on Unices we use the file handle.

@param[in]	path		File name
@param[in]	fh		File handle for the file - if opened
@return true if the file system supports sparse files */
bool os_is_sparse_file_supported(const char *path, pfs_os_file_t fh) {
  /* In this debugging mode, we act as if punch hole is supported,
  then we skip any calls to actually punch a hole.  In this way,
  Transparent Page Compression is still being tested. */
  DBUG_EXECUTE_IF("ignore_punch_hole", return (true););

#ifdef _WIN32
  return (os_is_sparse_file_supported_win32(path));
#else
  dberr_t err;

  /* We don't know the FS block size, use the sector size. The FS
  will do the magic. */
  err = os_file_punch_hole(fh.m_file, 0, UNIV_PAGE_SIZE);

  return (err == DB_SUCCESS);
#endif /* _WIN32 */
}
