#include <innodb/log_write/prepare_for_write_ahead.h>

#include <innodb/log_types/log_t.h>
#include <innodb/log_write/compute_next_write_ahead_end.h>
#include <innodb/log_write/current_file_has_space.h>
#include <innodb/log_redo/LOG_SYNC_POINT.h>


namespace Log_files_write_impl {

size_t prepare_for_write_ahead(log_t &log, uint64_t real_offset,
                                             size_t &write_size) {
  /* We need to perform write ahead during this write. */

  const auto next_wa = compute_next_write_ahead_end(real_offset);

  ut_a(real_offset + write_size <= next_wa);

  size_t write_ahead =
      static_cast<size_t>(next_wa - (real_offset + write_size));

  if (!current_file_has_space(log, real_offset, write_size + write_ahead)) {
    /* We must not write further than to the end
    of the current log file.

    Note, that: log.file_size - LOG_FILE_HDR_SIZE
    does not have to be divisible by size of write
    ahead. Example given:
            innodb_log_file_size = 1024M,
            innodb_log_write_ahead_size = 4KiB,
            LOG_FILE_HDR_SIZE is 2KiB. */

    write_ahead = static_cast<size_t>(log.current_file_end_offset -
                                      real_offset - write_size);
  }

  ut_a(current_file_has_space(log, real_offset, write_size + write_ahead));

  LOG_SYNC_POINT("log_writer_before_write_ahead");

  std::memset(log.write_ahead_buf + write_size, 0x00, write_ahead);

  write_size += write_ahead;

  return (write_ahead);
}


}
