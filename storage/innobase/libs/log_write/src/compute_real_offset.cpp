#include <innodb/log_write/compute_real_offset.h>

#include <innodb/log_types/log_t.h>
#include <innodb/log_types/flags.h>
#include <innodb/log_redo/log_files_real_offset_for_lsn.h>

namespace Log_files_write_impl {

uint64_t compute_real_offset(const log_t &log, lsn_t start_lsn) {
  ut_a(start_lsn >= log.current_file_lsn);

  ut_a(log.current_file_real_offset % log.file_size >= LOG_FILE_HDR_SIZE);

  const auto real_offset =
      log.current_file_real_offset + (start_lsn - log.current_file_lsn);

  ut_a(real_offset % log.file_size >= LOG_FILE_HDR_SIZE ||
       real_offset == log.current_file_end_offset);

  ut_a(real_offset % OS_FILE_LOG_BLOCK_SIZE == 0);

  ut_a(log_files_real_offset_for_lsn(log, start_lsn) ==
           real_offset % log.files_real_capacity ||
       real_offset == log.current_file_end_offset);

  return (real_offset);
}

}
