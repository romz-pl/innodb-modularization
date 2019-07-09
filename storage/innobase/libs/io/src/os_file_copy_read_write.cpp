#include <innodb/io/os_file_copy_read_write.h>

#include <innodb/io/IORequest.h>
#include <innodb/io/os_file_read_func.h>
#include <innodb/io/os_file_write_func.h>
#include <innodb/align/ut_align.h>

/** copy data from one file to another file using read, write.
@param[in]	src_file	file handle to copy from
@param[in]	src_offset	offset to copy from
@param[in]	dest_file	file handle to copy to
@param[in]	dest_offset	offset to copy to
@param[in]	size		number of bytes to copy
@return DB_SUCCESS if successful */
dberr_t os_file_copy_read_write(os_file_t src_file,
                                       os_offset_t src_offset,
                                       os_file_t dest_file,
                                       os_offset_t dest_offset, uint size) {
  dberr_t err;
  uint request_size;
  const uint BUF_SIZE = 4 * UNIV_SECTOR_SIZE;

  char buf[BUF_SIZE + UNIV_SECTOR_SIZE];
  char *buf_ptr;

  buf_ptr = static_cast<char *>(ut_align(buf, UNIV_SECTOR_SIZE));

  IORequest read_request(IORequest::READ);
  read_request.disable_compression();
  read_request.clear_encrypted();

  IORequest write_request(IORequest::WRITE);
  write_request.disable_compression();
  write_request.clear_encrypted();

  while (size > 0) {
    if (size > BUF_SIZE) {
      request_size = BUF_SIZE;
    } else {
      request_size = size;
    }

    err = os_file_read_func(read_request, src_file, buf_ptr, src_offset,
                            request_size);

    if (err != DB_SUCCESS) {
      return (err);
    }
    src_offset += request_size;

    err = os_file_write_func(write_request, "file copy", dest_file, buf_ptr,
                             dest_offset, request_size);

    if (err != DB_SUCCESS) {
      return (err);
    }
    dest_offset += request_size;
    size -= request_size;
  }

  return (DB_SUCCESS);
}
