#include <innodb/log_block/log_block_set_checksum.h>

#include <innodb/log_types/flags.h>
#include <innodb/machine/data.h>

void log_block_set_checksum(byte *log_block, uint32_t checksum) {
  mach_write_to_4(log_block + OS_FILE_LOG_BLOCK_SIZE - LOG_BLOCK_CHECKSUM,
                  checksum);
}
