#include <innodb/log_write/update_current_write_ahead.h>

#include <innodb/log_types/log_t.h>
#include <innodb/align/ut_uint64_align_down.h>
#include <innodb/log_write/srv_log_write_ahead_size.h>

namespace Log_files_write_impl {

void update_current_write_ahead(log_t &log, uint64_t real_offset,
                                              size_t write_size) {
  const auto end = real_offset + write_size;

  if (end > log.write_ahead_end_offset) {
    log.write_ahead_end_offset =
        ut_uint64_align_down(end, srv_log_write_ahead_size);
  }
}

}
