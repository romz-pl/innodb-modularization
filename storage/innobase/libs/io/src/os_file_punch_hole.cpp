#include <innodb/io/os_file_punch_hole.h>

#include <innodb/io/os_file_punch_hole_posix.h>

/** Free storage space associated with a section of the file.
@param[in]	fh		Open file handle
@param[in]	off		Starting offset (SEEK_SET)
@param[in]	len		Size of the hole
@return DB_SUCCESS or error code */
dberr_t os_file_punch_hole(os_file_t fh, os_offset_t off, os_offset_t len) {
  /* In this debugging mode, we act as if punch hole is supported,
  and then skip any calls to actually punch a hole here.
  In this way, Transparent Page Compression is still being tested. */
  DBUG_EXECUTE_IF("ignore_punch_hole", return (DB_SUCCESS););

#ifdef _WIN32
  return (os_file_punch_hole_win32(fh, off, len));
#else
  return (os_file_punch_hole_posix(fh, off, len));
#endif /* _WIN32 */
}
