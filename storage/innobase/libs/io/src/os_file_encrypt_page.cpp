#include <innodb/io/os_file_encrypt_page.h>

#include <innodb/align/ut_align.h>
#include <innodb/io/Encryption.h>
#include <innodb/io/os_alloc_block.h>

/** Disk sector size of aligning write buffer for DIRECT_IO */
static const ulint os_io_ptr_align = UNIV_SECTOR_SIZE;

/** Encrypt a page content when write it to disk.
@param[in]	type		IO flags
@param[out]	buf		buffer to read or write
@param[in,out]	n		number of bytes to read/write, starting from
                                offset
@return pointer to the encrypted page */
Block *os_file_encrypt_page(const IORequest &type, void *&buf,
                                   ulint *n) {
  byte *encrypted_page;
  ulint encrypted_len = *n;
  byte *buf_ptr;
  Encryption encryption(type.encryption_algorithm());

  ut_ad(type.is_write());
  ut_ad(type.is_encrypted());

  Block *block = os_alloc_block();

  encrypted_page = static_cast<byte *>(ut_align(block->m_ptr, os_io_ptr_align));

  buf_ptr = encryption.encrypt(type, reinterpret_cast<byte *>(buf), *n,
                               encrypted_page, &encrypted_len);

  bool encrypted = buf_ptr != buf;

  if (encrypted) {
    buf = buf_ptr;
    *n = encrypted_len;
  }

  return (block);
}

