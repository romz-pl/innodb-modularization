#include <innodb/io/os_file_write_func.h>

#include <innodb/io/os_file_write_page.h>
#include <innodb/io/IORequest.h>

/** NOTE! Use the corresponding macro os_file_write(), not directly
Requests a synchronous write operation.
@param[in]	type		IO flags
@param[in]	name		name of the file or path as a null-terminated
                                string
@param[in]	file		handle to an open file
@param[out]	buf		buffer from which to write
@param[in]	offset		file offset from the start where to read
@param[in]	n		number of bytes to read, starting from offset
@return DB_SUCCESS if request was successful, false if fail */
dberr_t os_file_write_func(IORequest &type, const char *name, os_file_t file,
                           const void *buf, os_offset_t offset, ulint n) {
  ut_ad(type.validate());
  ut_ad(type.is_write());

  /* We never compress the first page.
  Note: This assumes we always do block IO. */
  if (offset == 0) {
    type.clear_compressed();
  }

  const byte *ptr = reinterpret_cast<const byte *>(buf);

  return (os_file_write_page(type, name, file, ptr, offset, n));
}
