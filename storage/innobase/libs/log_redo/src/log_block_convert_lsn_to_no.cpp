#include <innodb/log_redo/log_block_convert_lsn_to_no.h>

#include <innodb/log_redo/flags.h>
#include <innodb/machine/data.h>

uint32_t log_block_convert_lsn_to_no(lsn_t lsn) {
  return ((uint32_t)(lsn / OS_FILE_LOG_BLOCK_SIZE) % LOG_BLOCK_MAX_NO + 1);
}
