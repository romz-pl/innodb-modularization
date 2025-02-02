#include <innodb/log_block/log_block_get_checkpoint_no.h>

#include <innodb/log_types/flags.h>
#include <innodb/machine/data.h>

uint32_t log_block_get_checkpoint_no(const byte *log_block) {
  return (mach_read_from_4(log_block + LOG_BLOCK_CHECKPOINT_NO));
}
