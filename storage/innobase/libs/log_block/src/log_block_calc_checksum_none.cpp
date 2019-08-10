#include <innodb/log_block/log_block_calc_checksum_none.h>

#include <innodb/log_types/flags.h>

uint32_t log_block_calc_checksum_none(const byte *log_block) {
  return (LOG_NO_CHECKSUM_MAGIC);
}
