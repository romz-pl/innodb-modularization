#include <innodb/log_files/log_files_real_offset_for_lsn.h>

#include <innodb/log_types/flags.h>
#include <innodb/log_files/log_files_real_offset.h>
#include <innodb/log_files/log_files_size_offset.h>
#include <innodb/log_types/log_writer_mutex_own.h>
#include <innodb/log_types/log_t.h>

uint64_t log_files_real_offset_for_lsn(const log_t &log, lsn_t lsn) {
  uint64_t size_offset;
  uint64_t size_capacity;
  uint64_t delta;

  ut_ad(log_writer_mutex_own(log));

  size_capacity = log.n_files * (log.file_size - LOG_FILE_HDR_SIZE);

  if (lsn >= log.current_file_lsn) {
    delta = lsn - log.current_file_lsn;

    delta = delta % size_capacity;

  } else {
    /* Special case because lsn and offset are unsigned. */

    delta = log.current_file_lsn - lsn;

    delta = size_capacity - delta % size_capacity;
  }

  size_offset = log_files_size_offset(log, log.current_file_real_offset);

  size_offset = (size_offset + delta) % size_capacity;

  return (log_files_real_offset(log, size_offset));
}
