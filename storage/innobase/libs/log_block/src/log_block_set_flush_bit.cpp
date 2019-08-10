#include <innodb/log_block/log_block_set_flush_bit.h>

#include <innodb/log_types/flags.h>
#include <innodb/machine/data.h>
#include <innodb/assert/assert.h>

void log_block_set_flush_bit(byte *log_block, bool value) {
  uint32_t field = mach_read_from_4(log_block + LOG_BLOCK_HDR_NO);

  ut_a(field != 0);

  if (value) {
    field = field | LOG_BLOCK_FLUSH_BIT_MASK;
  } else {
    field = field & ~LOG_BLOCK_FLUSH_BIT_MASK;
  }

  mach_write_to_4(log_block + LOG_BLOCK_HDR_NO, field);
}
