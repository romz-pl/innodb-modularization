#include <innodb/tablespace/os_file_read_first_page_func.h>

#include <innodb/io/IORequest.h>
#include <innodb/io/os_file_read_page.h>
#include <innodb/tablespace/fsp_header_get_flags.h>
#include <innodb/disk/page_size_t.h>

/** NOTE! Use the corresponding macro os_file_read_first_page(), not
directly this function!
Requests a synchronous positioned read operation of page 0 of IBD file
@return DB_SUCCESS if request was successful, DB_IO_ERROR on failure
@param[in]	type		IO flags
@param[in]	file		handle to an open file
@param[out]	buf		buffer where to read
@param[in]	n		number of bytes to read, starting from offset
@return DB_SUCCESS or error code */
dberr_t os_file_read_first_page_func(IORequest &type, os_file_t file, void *buf,
                                     ulint n) {
  ut_ad(type.is_read());

  dberr_t err =
      os_file_read_page(type, file, buf, 0, UNIV_ZIP_SIZE_MIN, NULL, true);

  if (err == DB_SUCCESS) {
    uint32_t flags = fsp_header_get_flags(static_cast<byte *>(buf));
    const page_size_t page_size(flags);
    ut_ad(page_size.physical() <= n);
    err =
        os_file_read_page(type, file, buf, 0, page_size.physical(), NULL, true);
  }
  return (err);
}
