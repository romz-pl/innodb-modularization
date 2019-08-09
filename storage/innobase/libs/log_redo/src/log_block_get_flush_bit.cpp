#include <innodb/log_redo/log_block_get_flush_bit.h>

#include <innodb/log_types/flags.h>
#include <innodb/machine/data.h>

bool log_block_get_flush_bit(const byte *log_block) {
  if (LOG_BLOCK_FLUSH_BIT_MASK &
      mach_read_from_4(log_block + LOG_BLOCK_HDR_NO)) {
    return (true);
  }

  return (false);
}
