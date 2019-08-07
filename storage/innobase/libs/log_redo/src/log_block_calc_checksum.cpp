#include <innodb/log_redo/log_block_calc_checksum.h>

#include <innodb/log_redo/flags.h>
#include <innodb/machine/data.h>
#include <innodb/log_redo/log_checksum_algorithm_ptr.h>

uint32_t log_block_calc_checksum(const byte *log_block) {
  return (log_checksum_algorithm_ptr.load()(log_block));
}
