#include <innodb/log_write/validate_start_lsn.h>

#include <innodb/log_types/log_t.h>
#include <innodb/log_types/flags.h>

namespace Log_files_write_impl {

void validate_start_lsn(const log_t &log, lsn_t start_lsn,
                                      size_t buffer_size) {
  /* start_lsn corresponds to block, it must be aligned to 512 */
  ut_a(start_lsn % OS_FILE_LOG_BLOCK_SIZE == 0);

  /* Either full log block writes are possible or partial writes,
  which have to cover full header of log block then. */
  ut_a((start_lsn + buffer_size) % OS_FILE_LOG_BLOCK_SIZE >=
           LOG_BLOCK_HDR_SIZE ||
       (start_lsn + buffer_size) % OS_FILE_LOG_BLOCK_SIZE == 0);

  /* Partial writes do not touch footer of log block. */
  ut_a((start_lsn + buffer_size) % OS_FILE_LOG_BLOCK_SIZE <
       OS_FILE_LOG_BLOCK_SIZE - LOG_BLOCK_TRL_SIZE);

  /* There are no holes. Note that possibly start_lsn is smaller,
  because it always points to the beginning of log block. */
  ut_a(start_lsn <= log.write_lsn.load());
}

}
