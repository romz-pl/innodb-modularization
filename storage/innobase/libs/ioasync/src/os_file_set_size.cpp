#include <innodb/ioasync/os_file_set_size.h>

#include <innodb/align/ut_align.h>
#include <innodb/allocator/ut_free.h>
#include <innodb/allocator/ut_malloc_nokey.h>
#include <innodb/assert/assert.h>
#include <innodb/io/IORequest.h>
#include <innodb/io/os_file_flush.h>
#include <innodb/io/os_fsync_threshold.h>
#include <innodb/ioasync/os_aio.h>
#include <innodb/logger/info.h>
#include <innodb/io/os_file_flush.h>

#include <algorithm>
#include <stdio.h>

/**  Write the specified number of zeros to a file from specific offset.
@param[in]	name		name of the file or path as a null-terminated
                                string
@param[in]	file		handle to a file
@param[in]	offset		file offset
@param[in]	size		file size
@param[in]	read_only	Enable read-only checks if true
@param[in]	flush		Flush file content to disk
@return true if success */
bool os_file_set_size(const char *name, pfs_os_file_t file, os_offset_t offset,
                      os_offset_t size, bool read_only, bool flush) {
  /* Write up to FSP_EXTENT_SIZE bytes at a time. */
  ulint buf_size = 0;

  if (size <= UNIV_PAGE_SIZE) {
    buf_size = 1;
  } else {
    buf_size = std::min(static_cast<ulint>(64),
                      static_cast<ulint>(size / UNIV_PAGE_SIZE));
  }

  ut_ad(buf_size != 0);

  buf_size *= UNIV_PAGE_SIZE;

  /* Align the buffer for possible raw i/o */
  byte *buf2;

  buf2 = static_cast<byte *>(ut_malloc_nokey(buf_size + UNIV_PAGE_SIZE));

  byte *buf = static_cast<byte *>(ut_align(buf2, UNIV_PAGE_SIZE));

  /* Write buffer full of zeros */
  memset(buf, 0, buf_size);

  if (size >= (os_offset_t)100 << 20) {
    ib::info(ER_IB_MSG_826) << "Progress in MB:";
  }

  os_offset_t current_size = offset;

  while (current_size < size) {
    ulint n_bytes;

    if (size - current_size < (os_offset_t)buf_size) {
      n_bytes = (ulint)(size - current_size);
    } else {
      n_bytes = buf_size;
    }

    dberr_t err;
    IORequest request(IORequest::WRITE);

#ifdef UNIV_HOTBACKUP

    err = os_file_write(request, name, file, buf, current_size, n_bytes);
#else
    /* Using AIO_mode::SYNC mode on POSIX systems will result in
    fall back to os_file_write/read. On Windows it will use
    special mechanism to wait before it returns back. */

    err = os_aio(request, AIO_mode::SYNC, name, file, buf, current_size,
                 n_bytes, read_only, NULL, NULL);
#endif /* UNIV_HOTBACKUP */

    if (err != DB_SUCCESS) {
      ut_free(buf2);
      return (false);
    }

    /* Print about progress for each 100 MB written */
    if ((current_size + n_bytes) / (100 << 20) != current_size / (100 << 20)) {
      fprintf(stderr, " %lu00",
              (ulong)((current_size + n_bytes) / (100 << 20)));
    }

    /* Flush after each os_fsync_threhold bytes */
    if (flush && os_fsync_threshold != 0) {
      if ((current_size + n_bytes) / os_fsync_threshold !=
          current_size / os_fsync_threshold) {
        DBUG_EXECUTE_IF("flush_after_reaching_threshold",
                        std::cerr << os_fsync_threshold
                                  << " bytes being flushed at once"
                                  << std::endl;);

        bool ret = os_file_flush(file);

        if (!ret) {
          ut_free(buf2);
          return (false);
        }
      }
    }

    current_size += n_bytes;
  }

  if (size >= (os_offset_t)100 << 20) {
    fprintf(stderr, "\n");
  }

  ut_free(buf2);

  if (flush) {
    return (os_file_flush(file));
  }

  return (true);
}


