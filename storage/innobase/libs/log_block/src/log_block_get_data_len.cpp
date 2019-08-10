#include <innodb/log_block/log_block_get_data_len.h>

#include <innodb/log_types/flags.h>
#include <innodb/machine/data.h>

uint32_t log_block_get_data_len(const byte *log_block) {
  return (mach_read_from_2(log_block + LOG_BLOCK_HDR_DATA_LEN));
}
