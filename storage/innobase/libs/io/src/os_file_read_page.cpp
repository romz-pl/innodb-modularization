#include <innodb/io/os_file_read_page.h>

#include <innodb/assert/assert.h>
#include <innodb/io/Compression.h>
#include <innodb/io/IORequest.h>
#include <innodb/io/os_bytes_read_since_printout.h>
#include <innodb/io/os_file_handle_error.h>
#include <innodb/io/os_file_handle_error_no_exit.h>
#include <innodb/io/os_file_pread.h>
#include <innodb/logger/error.h>
#include <innodb/logger/fatal.h>

/** Requests a synchronous positioned read operation.
@return DB_SUCCESS if request was successful, false if fail
@param[in]	type		IO flags
@param[in]	file		handle to an open file
@param[out]	buf		buffer where to read
@param[in]	offset		file offset from the start where to read
@param[in]	n		number of bytes to read, starting from offset
@param[out]	o		number of bytes actually read
@param[in]	exit_on_err	if true then exit on error
@return DB_SUCCESS or error code */
MY_ATTRIBUTE((warn_unused_result)) dberr_t
    os_file_read_page(IORequest &type, os_file_t file, void *buf,
                      os_offset_t offset, ulint n, ulint *o, bool exit_on_err) {
  dberr_t err;

#ifdef UNIV_HOTBACKUP
  static meb::Mutex meb_mutex;

  meb_mutex.lock();
#endif /* UNIV_HOTBACKUP */
  os_bytes_read_since_printout += n;
#ifdef UNIV_HOTBACKUP
  meb_mutex.unlock();
#endif /* UNIV_HOTBACKUP */

  ut_ad(type.validate());
  ut_ad(n > 0);

  for (;;) {
    ssize_t n_bytes;

    n_bytes = os_file_pread(type, file, buf, n, offset, &err);

    if (o != NULL) {
      *o = n_bytes;
    }

    if (err != DB_SUCCESS && !exit_on_err) {
      return (err);

    } else if ((ulint)n_bytes == n) {
      /** The read will succeed but decompress can fail
      for various reasons. */

      if (type.is_compression_enabled() &&
          !Compression::is_compressed_page(static_cast<byte *>(buf))) {
        return (DB_SUCCESS);

      } else {
        return (err);
      }
    }

    ib::error(ER_IB_MSG_817)
        << "Tried to read " << n << " bytes at offset " << offset
        << ", but was only able to read " << n_bytes;

    if (exit_on_err) {
      if (!os_file_handle_error(NULL, "read")) {
        /* Hard error */
        break;
      }

    } else if (!os_file_handle_error_no_exit(NULL, "read", false)) {
      /* Hard error */
      break;
    }

    if (n_bytes > 0 && (ulint)n_bytes < n) {
      n -= (ulint)n_bytes;
      offset += (ulint)n_bytes;
      buf = reinterpret_cast<uchar *>(buf) + (ulint)n_bytes;
    }
  }

  ib::fatal(ER_IB_MSG_818) << "Cannot read from file. OS error number " << errno
                           << ".";

  return (err);
}


