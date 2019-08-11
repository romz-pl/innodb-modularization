#include <innodb/log_write/compute_next_write_ahead_end.h>

#include <innodb/log_types/log_t.h>
#include <innodb/align/ut_uint64_align_down.h>
#include <innodb/log_write/srv_log_write_ahead_size.h>

namespace Log_files_write_impl {

uint64_t compute_next_write_ahead_end(uint64_t real_offset) {
  const auto last_wa =
      ut_uint64_align_down(real_offset, srv_log_write_ahead_size);

  const auto next_wa = last_wa + srv_log_write_ahead_size;

  ut_a(next_wa > real_offset);
  ut_a(next_wa % srv_log_write_ahead_size == 0);

  return (next_wa);
}

}
