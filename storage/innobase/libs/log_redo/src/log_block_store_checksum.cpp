#include <innodb/log_redo/log_block_store_checksum.h>

#include <innodb/log_types/log_block_set_checksum.h>
#include <innodb/log_types/log_block_calc_checksum.h>

void log_block_store_checksum(byte *log_block) {
  log_block_set_checksum(log_block, log_block_calc_checksum(log_block));
}
