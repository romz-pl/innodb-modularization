#include <innodb/tablespace/fil_write_zeros.h>

#include <innodb/assert/assert.h>
#include <innodb/allocator/ut_zalloc_nokey.h>
#include <innodb/ioasync/os_aio_func.h>
#include <innodb/align/ut_align.h>
#include <innodb/allocator/ut_free.h>
#include <innodb/io/IORequest.h>
#include <innodb/tablespace/fil_node_t.h>

#include <algorithm>

/** Fill the pages with NULs
@param[in] file		Tablespace file
@param[in] page_size	physical page size
@param[in] start	Offset from the start of the file in bytes
@param[in] len		Length in bytes
@param[in] read_only_mode
                        if true, then read only mode checks are enforced.
@return DB_SUCCESS or error code */
dberr_t fil_write_zeros(const fil_node_t *file, ulint page_size,
                               os_offset_t start, ulint len,
                               bool read_only_mode) {
  ut_a(len > 0);

  /* Extend at most 1M at a time */
  ulint n_bytes = std::min(static_cast<ulint>(1024 * 1024), len);

  byte *ptr = reinterpret_cast<byte *>(ut_zalloc_nokey(n_bytes + page_size));

  byte *buf = reinterpret_cast<byte *>(ut_align(ptr, page_size));

  os_offset_t offset = start;
  dberr_t err = DB_SUCCESS;
  const os_offset_t end = start + len;
  IORequest request(IORequest::WRITE);

  while (offset < end) {
#ifdef UNIV_HOTBACKUP
    err =
        os_file_write(request, file->name, file->handle, buf, offset, n_bytes);
#else  /* UNIV_HOTBACKUP */
    err = os_aio_func(request, AIO_mode::SYNC, file->name, file->handle, buf,
                      offset, n_bytes, read_only_mode, nullptr, nullptr);
#endif /* UNIV_HOTBACKUP */

    if (err != DB_SUCCESS) {
      break;
    }

    offset += n_bytes;

    n_bytes = std::min(n_bytes, static_cast<ulint>(end - offset));

    DBUG_EXECUTE_IF("ib_crash_during_tablespace_extension", DBUG_SUICIDE(););
  }

  ut_free(ptr);

  return (err);
}

