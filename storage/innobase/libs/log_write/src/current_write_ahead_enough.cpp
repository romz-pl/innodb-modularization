#include <innodb/log_write/current_write_ahead_enough.h>

#include <innodb/log_types/log_t.h>
#include <innodb/log_write/write_ahead_enough.h>

namespace Log_files_write_impl {

bool current_write_ahead_enough(const log_t &log, uint64_t offset,
                                              size_t size) {
  return (write_ahead_enough(log.write_ahead_end_offset, offset, size));
}

}
