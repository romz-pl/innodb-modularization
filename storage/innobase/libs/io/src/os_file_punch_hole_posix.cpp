#include <innodb/io/os_file_punch_hole_posix.h>

#include <innodb/logger/warn.h>

#include <fcntl.h>

/** Free storage space associated with a section of the file.
@param[in]	fh		Open file handle
@param[in]	off		Starting offset (SEEK_SET)
@param[in]	len		Size of the hole
@return DB_SUCCESS or error code */
dberr_t os_file_punch_hole_posix(os_file_t fh, os_offset_t off,
                                        os_offset_t len) {
#ifdef HAVE_FALLOC_PUNCH_HOLE_AND_KEEP_SIZE
  const int mode = FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE;

  int ret = fallocate(fh, mode, off, len);

  if (ret == 0) {
    return (DB_SUCCESS);
  }

  ut_a(ret == -1);

  if (errno == ENOTSUP) {
    return (DB_IO_NO_PUNCH_HOLE);
  }

  ib::warn(ER_IB_MSG_754) << "fallocate(" << fh
                          << ", FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE, "
                          << off << ", " << len
                          << ") returned errno: " << errno;

  return (DB_IO_ERROR);

#elif defined(UNIV_SOLARIS)

// Use F_FREESP

#endif /* HAVE_FALLOC_PUNCH_HOLE_AND_KEEP_SIZE */

  return (DB_IO_NO_PUNCH_HOLE);
}
