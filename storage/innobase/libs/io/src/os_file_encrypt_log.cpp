#include <innodb/io/os_file_encrypt_log.h>

#include <innodb/allocator/ut_malloc_nokey.h>
#include <innodb/align/ut_align.h>
#include <innodb/io/block_cache.h>
#include <innodb/io/os_alloc_block.h>
#include <innodb/io/os_io_ptr_align.h>


/** Encrypt log blocks content when write it to disk.
@param[in]	type		IO flags
@param[in,out]	buf		buffer to read or write
@param[in,out]	scratch		buffer for encrypting log
@param[in,out]	n		number of bytes to read/write, starting from
                                offset
@return pointer to the encrypted log blocks */
Block *os_file_encrypt_log(const IORequest &type, void *&buf,
                                  byte *&scratch, ulint *n) {
  byte *encrypted_log;
  ulint encrypted_len = *n;
  byte *buf_ptr;
  Encryption encryption(type.encryption_algorithm());
  Block *block = NULL;

  ut_ad(type.is_write() && type.is_encrypted() && type.is_log());
  ut_ad(*n % OS_FILE_LOG_BLOCK_SIZE == 0);

  if (*n <= BUFFER_BLOCK_SIZE - os_io_ptr_align) {
    block = os_alloc_block();
    buf_ptr = block->m_ptr;
    scratch = NULL;
  } else {
    buf_ptr = static_cast<byte *>(ut_malloc_nokey(*n + os_io_ptr_align));
    scratch = buf_ptr;
  }

  encrypted_log = static_cast<byte *>(ut_align(buf_ptr, os_io_ptr_align));

  encrypted_log = encryption.encrypt_log(type, reinterpret_cast<byte *>(buf),
                                         *n, encrypted_log, &encrypted_len);

  bool encrypted = encrypted_log != buf;

  if (encrypted) {
    buf = encrypted_log;
    *n = encrypted_len;
  }

  return (block);
}

