#include <innodb/log_types/log_block_calc_checksum_crc32.h>

#include <innodb/log_types/flags.h>
#include <innodb/machine/data.h>
#include <innodb/crc32/crc32.h>

uint32_t log_block_calc_checksum_crc32(const byte *log_block) {
  return (ut_crc32(log_block, OS_FILE_LOG_BLOCK_SIZE - LOG_BLOCK_TRL_SIZE));
}
