#include <innodb/log_redo/log_block_get_encrypt_bit.h>

#include <innodb/machine/data.h>
#include <innodb/log_types/flags.h>

bool log_block_get_encrypt_bit(const byte *log_block) {
  if (LOG_BLOCK_ENCRYPT_BIT_MASK &
      mach_read_from_2(log_block + LOG_BLOCK_HDR_DATA_LEN)) {
    return (true);
  }

  return (false);
}
