#include <innodb/io/os_file_copy_func.h>

#include <innodb/io/os_file_seek.h>
#include <innodb/io/os_file_copy_read_write.h>
#include <innodb/io/OS_FD_FROM_FILE.h>
#include <innodb/logger/info.h>

#include <sys/sendfile.h>

/** copy data from one file to another file.
@param[in]	src_file	file handle to copy from
@param[in]	src_offset	offset to copy from
@param[in]	dest_file	file handle to copy to
@param[in]	dest_offset	offset to copy to
@param[in]	size		number of bytes to copy
@return DB_SUCCESS if successful */
#ifdef __linux__
dberr_t os_file_copy_func(os_file_t src_file, os_offset_t src_offset,
                          os_file_t dest_file, os_offset_t dest_offset,
                          uint size) {
  dberr_t err;
  static bool use_sendfile = true;

  uint actual_size;
  int ret_size;

  int src_fd;
  int dest_fd;

  if (!os_file_seek(nullptr, src_file, src_offset)) {
    return (DB_IO_ERROR);
  }

  if (!os_file_seek(nullptr, dest_file, dest_offset)) {
    return (DB_IO_ERROR);
  }

  src_fd = OS_FD_FROM_FILE(src_file);
  dest_fd = OS_FD_FROM_FILE(dest_file);

  while (use_sendfile && size > 0) {
    ret_size = sendfile(dest_fd, src_fd, nullptr, size);

    if (ret_size == -1) {
      /* Fall through read/write path. */
      ib::info(ER_IB_MSG_827) << "sendfile failed to copy data"
                                 " : trying read/write ";

      use_sendfile = false;
      break;
    }

    actual_size = static_cast<uint>(ret_size);

    ut_ad(size >= actual_size);
    size -= actual_size;
  }

  if (size == 0) {
    return (DB_SUCCESS);
  }

  err = os_file_copy_read_write(src_file, src_offset, dest_file, dest_offset,
                                size);

  return (err);
}
#else
dberr_t os_file_copy_func(os_file_t src_file, os_offset_t src_offset,
                          os_file_t dest_file, os_offset_t dest_offset,
                          uint size) {
  dberr_t err;

  err = os_file_copy_read_write(src_file, src_offset, dest_file, dest_offset,
                                size);
  return (err);
}
#endif
