#include <innodb/tablespace/fil_io.h>

#include <innodb/tablespace/fil_system.h>

/** Read or write data from a file.
@param[in]	type		IO context
@param[in]	sync		If true then do synchronous IO
@param[in]	page_id		page id
@param[in]	page_size	page size
@param[in]	byte_offset	remainder of offset in bytes; in aio this
                                must be divisible by the OS block size
@param[in]	len		how many bytes to read or write; this must
                                not cross a file boundary; in AIO this must
                                be a block size multiple
@param[in,out]	buf		buffer where to store read data or from where
                                to write; in AIO this must be appropriately
                                aligned
@param[in]	message		message for AIO handler if !sync, else ignored
@return error code
@retval DB_SUCCESS on success
@retval DB_TABLESPACE_DELETED if the tablespace does not exist */
dberr_t fil_io(const IORequest &type, bool sync, const page_id_t &page_id,
               const page_size_t &page_size, ulint byte_offset, ulint len,
               void *buf, void *message) {
  auto shard = fil_system->shard_by_id(page_id.space());

  return (shard->do_io(type, sync, page_id, page_size, byte_offset, len, buf,
                       message));
}
