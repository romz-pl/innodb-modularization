#include <innodb/log_redo/log_lsn_validate.h>

#include <innodb/log_types/flags.h>

bool log_lsn_validate(lsn_t lsn) {
  const uint32_t offset = lsn % OS_FILE_LOG_BLOCK_SIZE;

  return (lsn >= LOG_START_LSN && offset >= LOG_BLOCK_HDR_SIZE &&
          offset < OS_FILE_LOG_BLOCK_SIZE - LOG_BLOCK_TRL_SIZE);
}
