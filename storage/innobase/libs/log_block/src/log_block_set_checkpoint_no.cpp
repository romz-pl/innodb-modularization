#include <innodb/log_block/log_block_set_checkpoint_no.h>

#include <innodb/log_types/flags.h>
#include <innodb/machine/data.h>

void log_block_set_checkpoint_no(byte *log_block, uint64_t no) {
  mach_write_to_4(log_block + LOG_BLOCK_CHECKPOINT_NO, (uint32_t)no);
}
