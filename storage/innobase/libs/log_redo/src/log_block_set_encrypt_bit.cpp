#include <innodb/log_redo/log_block_set_encrypt_bit.h>

#include <innodb/log_redo/flags.h>
#include <innodb/machine/data.h>

void log_block_set_encrypt_bit(byte *log_block, ibool val) {
  uint32_t field;

  field = mach_read_from_2(log_block + LOG_BLOCK_HDR_DATA_LEN);

  if (val) {
    field = field | LOG_BLOCK_ENCRYPT_BIT_MASK;
  } else {
    field = field & ~LOG_BLOCK_ENCRYPT_BIT_MASK;
  }

  mach_write_to_2(log_block + LOG_BLOCK_HDR_DATA_LEN, field);
}
