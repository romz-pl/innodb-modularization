#include <innodb/io/os_file_flush_func.h>

#include <innodb/io/os_file_fsync_posix.h>
#include <innodb/logger/error.h>
#include <innodb/io/os_file_handle_error.h>
#include <innodb/error/ut_error.h>

/** TRUE if a raw partition is in use */
extern ibool srv_start_raw_disk_in_use;

/** NOTE! Use the corresponding macro os_file_flush(), not directly this
function!
Flushes the write buffers of a given file to the disk.
@param[in]	file		handle to a file
@return true if success */
bool os_file_flush_func(os_file_t file) {
  int ret;

  ret = os_file_fsync_posix(file);

  if (ret == 0) {
    return (true);
  }

  /* Since Linux returns EINVAL if the 'file' is actually a raw device,
  we choose to ignore that error if we are using raw disks */

  if (srv_start_raw_disk_in_use && errno == EINVAL) {
    return (true);
  }

  ib::error(ER_IB_MSG_775) << "The OS said file flush did not succeed";

  os_file_handle_error(NULL, "flush");

  /* It is a fatal error if a file flush does not succeed, because then
  the database can get corrupt on disk */
  ut_error;

  return (false);
}

