#include <innodb/tablespace/fil_redo_io.h>

#include <innodb/io/IORequest.h>
#include <innodb/page/page_id_t.h>
#include <innodb/tablespace/fil_system.h>

/** Read or write redo log data (synchronous buffered IO).
@param[in]	type		IO context
@param[in]	page_id		where to read or write
@param[in]	page_size	page size
@param[in]	byte_offset	remainder of offset in bytes
@param[in]	len		this must not cross a file boundary;
@param[in,out]	buf		buffer where to store read data or from where
                                to write
@retval DB_SUCCESS if all OK */
dberr_t fil_redo_io(const IORequest &type, const page_id_t &page_id,
                    const page_size_t &page_size, ulint byte_offset, ulint len,
                    void *buf) {
  ut_ad(type.is_log());

  auto shard = fil_system->shard_by_id(page_id.space());
#if defined(_WIN32) && defined(WIN_ASYNC_IO)
  /* On Windows we always open the redo log file in AIO mode. ie. we
  use the AIO API for the read/write even for sync IO. */
  return (shard->do_io(type, true, page_id, page_size, byte_offset, len, buf,
                       nullptr));
#else
  return (shard->do_redo_io(type, page_id, page_size, byte_offset, len, buf));
#endif /* _WIN32  || WIN_ASYNC_IO*/
}
