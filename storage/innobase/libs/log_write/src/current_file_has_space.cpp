#include <innodb/log_write/current_file_has_space.h>

#include <innodb/log_types/log_t.h>

namespace Log_files_write_impl {

bool current_file_has_space(const log_t &log, uint64_t offset,
                                          size_t size) {
  return (offset + size <= log.current_file_end_offset);
}

}
