#include <innodb/log_write/log_file_header_fill_encryption.h>

#include <innodb/io/Encryption.h>
#include <innodb/log_redo/flags.h>
#include <innodb/assert/assert.h>

bool log_file_header_fill_encryption(byte *buf, byte *key, byte *iv,
                                            bool is_boot) {
  byte encryption_info[ENCRYPTION_INFO_SIZE];

  if (!Encryption::fill_encryption_info(key, iv, encryption_info, is_boot)) {
    return (false);
  }

  ut_a(LOG_HEADER_CREATOR_END + ENCRYPTION_INFO_SIZE < OS_FILE_LOG_BLOCK_SIZE);

  memcpy(buf + LOG_HEADER_CREATOR_END, encryption_info, ENCRYPTION_INFO_SIZE);

  return (true);
}
