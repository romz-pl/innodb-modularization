#include <innodb/log_redo/log_block_set_first_rec_group.h>

#include <innodb/log_types/flags.h>
#include <innodb/machine/data.h>

void log_block_set_first_rec_group(byte *log_block, uint32_t offset) {
  mach_write_to_2(log_block + LOG_BLOCK_FIRST_REC_GROUP, offset);
}
