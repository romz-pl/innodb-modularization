#pragma once

#include <innodb/univ/univ.h>

#include <innodb/io/os_offset_t.h>
#include <innodb/io/os_file_t.h>
#include <innodb/io/IORequest.h>
#include <innodb/assert/assert.h>

struct Slot;

/** Helper class for doing synchronous file IO. Currently, the objective
is to hide the OS specific code, so that the higher level functions aren't
peppered with "#ifdef". Makes the code flow difficult to follow.  */
class SyncFileIO {
 public:
  /** Constructor
  @param[in]	fh	File handle
  @param[in,out]	buf	Buffer to read/write
  @param[in]	n	Number of bytes to read/write
  @param[in]	offset	Offset where to read or write */
  SyncFileIO(os_file_t fh, void *buf, ulint n, os_offset_t offset)
      : m_fh(fh), m_buf(buf), m_n(static_cast<ssize_t>(n)), m_offset(offset) {
    ut_ad(m_n > 0);
  }

  /** Destructor */
  ~SyncFileIO() { /* No op */
  }

  /** Do the read/write
  @param[in]	request	The IO context and type
  @return the number of bytes read/written or negative value on error */
  ssize_t execute(const IORequest &request);

  /** Do the read/write
  @param[in,out]	slot	The IO slot, it has the IO context
  @return the number of bytes read/written or negative value on error */
  static ssize_t execute(Slot *slot);

  /** Move the read/write offset up to where the partial IO succeeded.
  @param[in]	n_bytes	The number of bytes to advance */
  void advance(ssize_t n_bytes) {
    m_offset += n_bytes;

    ut_ad(m_n >= n_bytes);

    m_n -= n_bytes;

    m_buf = reinterpret_cast<uchar *>(m_buf) + n_bytes;
  }

 private:
  /** Open file handle */
  os_file_t m_fh;

  /** Buffer to read/write */
  void *m_buf;

  /** Number of bytes to read/write */
  ssize_t m_n;

  /** Offset from where to read/write */
  os_offset_t m_offset;
};

