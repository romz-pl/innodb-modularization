#include <innodb/log_redo/log_block_set_data_len.h>

#include <innodb/log_types/flags.h>
#include <innodb/machine/data.h>

void log_block_set_data_len(byte *log_block, ulint len) {
  mach_write_to_2(log_block + LOG_BLOCK_HDR_DATA_LEN, len);
}
