#include <innodb/log_write/start_next_file.h>

#include <innodb/log_types/log_t.h>
#include <innodb/log_files/log_files_header_flush.h>
#include <innodb/log_files/log_files_update_offsets.h>

namespace Log_files_write_impl {

void start_next_file(log_t &log, lsn_t start_lsn) {
  const auto before_update = log.current_file_end_offset;

  auto real_offset = before_update;

  ut_a(log.file_size % OS_FILE_LOG_BLOCK_SIZE == 0);
  ut_a(real_offset / log.file_size <= ULINT_MAX);

  ut_a(real_offset <= log.files_real_capacity);

  if (real_offset == log.files_real_capacity) {
    /* Wrapped log files, start at file 0,
    just after its initial headers. */
    real_offset = LOG_FILE_HDR_SIZE;
  }

  ut_a(real_offset + OS_FILE_LOG_BLOCK_SIZE <= log.files_real_capacity);

  /* Flush header of the new log file. */
  uint32_t nth_file = static_cast<uint32_t>(real_offset / log.file_size);
  log_files_header_flush(log, nth_file, start_lsn);

  /* Update following members of log:
  - current_file_lsn,
  - current_file_real_offset,
  - current_file_end_offset.
  The only reason is to optimize future calculations
  of offsets within the new log file. */
  log_files_update_offsets(log, start_lsn);

  ut_a(log.current_file_real_offset == before_update + LOG_FILE_HDR_SIZE ||
       (before_update == log.files_real_capacity &&
        log.current_file_real_offset == LOG_FILE_HDR_SIZE));

  ut_a(log.current_file_real_offset - LOG_FILE_HDR_SIZE ==
       log.current_file_end_offset - log.file_size);

  log.write_ahead_end_offset = 0;
}


}
