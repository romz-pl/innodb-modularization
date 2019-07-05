#include <innodb/io/os_file_io.h>

#include <innodb/allocator/ut_free.h>
#include <innodb/io/SyncFileIO.h>
#include <innodb/io/os_file_compress_page.h>
#include <innodb/io/os_file_encrypt_log.h>
#include <innodb/io/os_file_encrypt_page.h>
#include <innodb/io/os_file_io_complete.h>
#include <innodb/io/os_free_block.h>
#include <innodb/logger/warn.h>
#include <innodb/page/type.h>

/** Number of retries for partial I/O's */
static const ulint NUM_RETRIES_ON_PARTIAL_IO = 10;


/** Does a syncronous read or write depending upon the type specified
In case of partial reads/writes the function tries
NUM_RETRIES_ON_PARTIAL_IO times to read/write the complete data.
@param[in]	in_type		IO flags
@param[in]	file		handle to an open file
@param[out]	buf		buffer where to read
@param[in]	offset		file offset from the start where to read
@param[in]	n		number of bytes to read, starting from offset
@param[out]	err		DB_SUCCESS or error code
@return number of bytes read/written, -1 if error */
MY_ATTRIBUTE((warn_unused_result)) ssize_t
    os_file_io(const IORequest &in_type, os_file_t file, void *buf, ulint n,
               os_offset_t offset, dberr_t *err) {
  Block *block = NULL;
  ulint original_n = n;
  IORequest type = in_type;
  ssize_t bytes_returned = 0;
  byte *encrypt_log_buf = NULL;

  if (type.is_compressed()) {
    /* We don't compress the first page of any file. */
    ut_ad(offset > 0);

    block = os_file_compress_page(type, buf, &n);
  } else {
    block = NULL;
  }

  /* We do encryption after compression, since if we do encryption
  before compression, the encrypted data will cause compression fail
  or low compression rate. */
  if (type.is_encrypted() && type.is_write()) {
    if (!type.is_log()) {
      /* We don't encrypt the first page of any file. */
      Block *compressed_block = block;
      ut_ad(offset > 0);

      block = os_file_encrypt_page(type, buf, &n);

      if (compressed_block != NULL) {
        os_free_block(compressed_block);
      }
    } else {
      /* Skip encrypt log file header */
      if (offset >= LOG_FILE_HDR_SIZE) {
        block = os_file_encrypt_log(type, buf, encrypt_log_buf, &n);
      }
    }
  }

  SyncFileIO sync_file_io(file, buf, n, offset);

  for (ulint i = 0; i < NUM_RETRIES_ON_PARTIAL_IO; ++i) {
    ssize_t n_bytes = sync_file_io.execute(type);

    /* Check for a hard error. Not much we can do now. */
    if (n_bytes < 0) {
      break;

    } else if ((ulint)n_bytes + bytes_returned == n) {
      bytes_returned += n_bytes;

      if (offset > 0 && (type.is_compressed() || type.is_read())) {
        *err = os_file_io_complete(type, file, reinterpret_cast<byte *>(buf),
                                   NULL, original_n, offset, n);
      } else {
        *err = DB_SUCCESS;
      }

      if (block != NULL) {
        os_free_block(block);
      }

      if (encrypt_log_buf != NULL) {
        ut_free(encrypt_log_buf);
      }

      return (original_n);
    }

    /* Handle partial read/write. */

    ut_ad((ulint)n_bytes + bytes_returned < n);

    bytes_returned += (ulint)n_bytes;

    if (!type.is_partial_io_warning_disabled()) {
      const char *op = type.is_read() ? "read" : "written";

      ib::warn(ER_IB_MSG_812)
          << n << " bytes should have been " << op << ". Only "
          << bytes_returned << " bytes " << op << ". Retrying"
          << " for the remaining bytes.";
    }

    /* Advance the offset and buffer by n_bytes */
    sync_file_io.advance(n_bytes);
  }

  if (block != NULL) {
    os_free_block(block);
  }

  if (encrypt_log_buf != NULL) {
    ut_free(encrypt_log_buf);
  }

  *err = DB_IO_ERROR;

  if (!type.is_partial_io_warning_disabled()) {
    ib::warn(ER_IB_MSG_813)
        << "Retry attempts for " << (type.is_read() ? "reading" : "writing")
        << " partial data failed.";
  }

  return (bytes_returned);
}
