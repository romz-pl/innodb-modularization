#include <innodb/log_types/log_files_size_offset.h>

#include <innodb/log_types/log_t.h>
#include <innodb/log_types/flags.h>
#include <innodb/log_types/log_writer_mutex_own.h>

uint64_t log_files_size_offset(const log_t &log, uint64_t offset) {
  ut_ad(log_writer_mutex_own(log));

  return (offset - LOG_FILE_HDR_SIZE * (1 + offset / log.file_size));
}
