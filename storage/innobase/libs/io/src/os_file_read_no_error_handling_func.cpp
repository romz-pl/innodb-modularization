#include <innodb/io/os_file_read_no_error_handling_func.h>

#include <innodb/io/IORequest.h>
#include <innodb/io/os_file_read_page.h>

/** NOTE! Use the corresponding macro os_file_read_no_error_handling(),
not directly this function!
Requests a synchronous positioned read operation.
@return DB_SUCCESS if request was successful, DB_IO_ERROR on failure
@param[in]	type		IO flags
@param[in]	file		handle to an open file
@param[out]	buf		buffer where to read
@param[in]	offset		file offset from the start where to read
@param[in]	n		number of bytes to read, starting from offset
@param[out]	o		number of bytes actually read
@return DB_SUCCESS or error code */
dberr_t os_file_read_no_error_handling_func(IORequest &type, os_file_t file,
                                            void *buf, os_offset_t offset,
                                            ulint n, ulint *o) {
  ut_ad(type.is_read());

  return (os_file_read_page(type, file, buf, offset, n, o, false));
}
