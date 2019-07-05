#include <innodb/io/SyncFileIO.h>

#include <unistd.h>

/** Do the read/write
@param[in]	request	The IO context and type
@return the number of bytes read/written or negative value on error */
ssize_t SyncFileIO::execute(const IORequest &request) {
  ssize_t n_bytes;

  if (request.is_read()) {
    n_bytes = pread(m_fh, m_buf, m_n, m_offset);
  } else {
    ut_ad(request.is_write());
    n_bytes = pwrite(m_fh, m_buf, m_n, m_offset);
  }

  return (n_bytes);
}
