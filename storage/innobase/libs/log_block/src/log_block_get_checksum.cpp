#include <innodb/log_block/log_block_get_checksum.h>

#include <innodb/log_types/flags.h>
#include <innodb/machine/data.h>

uint32_t log_block_get_checksum(const byte *log_block) {
  return (mach_read_from_4(log_block + OS_FILE_LOG_BLOCK_SIZE -
                           LOG_BLOCK_CHECKSUM));
}
