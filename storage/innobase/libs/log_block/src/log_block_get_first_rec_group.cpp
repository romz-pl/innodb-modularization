#include <innodb/log_block/log_block_get_first_rec_group.h>

#include <innodb/log_types/flags.h>
#include <innodb/machine/data.h>

uint32_t log_block_get_first_rec_group(const byte *log_block) {
  return (mach_read_from_2(log_block + LOG_BLOCK_FIRST_REC_GROUP));
}
