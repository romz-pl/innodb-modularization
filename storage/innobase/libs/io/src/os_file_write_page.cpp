#include <innodb/io/os_file_write_page.h>

#include <innodb/logger/error.h>
#include <innodb/logger/info.h>
#include <innodb/io/os_file_pwrite.h>
#include <innodb/io/os_has_said_disk_full.h>

#include <cstring>


extern const char *OPERATING_SYSTEM_ERROR_MSG;

/** Requests a synchronous write operation.
@param[in]	type		IO flags
@param[in]	name		name of the file or path as a null-terminated
                                string
@param[in]	file		handle to an open file
@param[out]	buf		buffer from which to write
@param[in]	offset		file offset from the start where to read
@param[in]	n		number of bytes to read, starting from offset
@return DB_SUCCESS if request was successful, false if fail */
MY_ATTRIBUTE((warn_unused_result)) dberr_t
    os_file_write_page(IORequest &type, const char *name, os_file_t file,
                       const byte *buf, os_offset_t offset, ulint n) {
  dberr_t err;

  ut_ad(type.validate());
  ut_ad(n > 0);

  ssize_t n_bytes = os_file_pwrite(type, file, buf, n, offset, &err);

  if ((ulint)n_bytes != n && !os_has_said_disk_full) {
    ib::error(ER_IB_MSG_814) << "Write to file " << name << " failed at offset "
                             << offset << ", " << n
                             << " bytes should have been written,"
                                " only "
                             << n_bytes
                             << " were written."
                                " Operating system error number "
                             << errno
                             << "."
                                " Check that your OS and file system"
                                " support files of this size."
                                " Check also that the disk is not full"
                                " or a disk quota exceeded.";

    if (strerror(errno) != NULL) {
      ib::error(ER_IB_MSG_815)
          << "Error number " << errno << " means '" << strerror(errno) << "'";
    }

    ib::info(ER_IB_MSG_816) << OPERATING_SYSTEM_ERROR_MSG;

    os_has_said_disk_full = true;
  }

  return (err);
}
