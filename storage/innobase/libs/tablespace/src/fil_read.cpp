#include <innodb/tablespace/fil_read.h>

#include <innodb/tablespace/fil_io.h>
#include <innodb/io/IORequestRead.h>

/** Reads data from a space to a buffer. Remember that the possible incomplete
blocks at the end of file are ignored: they are not taken into account when
calculating the byte offset within a space.
@param[in]	page_id		page id
@param[in]	page_size	page size
@param[in]	byte_offset	remainder of offset in bytes; in aio this
must be divisible by the OS block size
@param[in]	len		how many bytes to read; this must not cross a
file boundary; in aio this must be a block size multiple
@param[in,out]	buf		buffer where to store data read; in aio this
must be appropriately aligned
@return DB_SUCCESS, or DB_TABLESPACE_DELETED if we are trying to do
i/o on a tablespace which does not exist */
dberr_t fil_read(const page_id_t &page_id, const page_size_t &page_size,
                        ulint byte_offset, ulint len, void *buf) {
  return (fil_io(IORequestRead, true, page_id, page_size, byte_offset, len, buf,
                 nullptr));
}
