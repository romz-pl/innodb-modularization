#include <innodb/log_write/validate_buffer.h>

#include <innodb/log_types/log_t.h>

namespace Log_files_write_impl {

void validate_buffer(const log_t &log, const byte *buffer,
                                   size_t buffer_size) {
  ut_a(buffer >= log.buf);
  ut_a(buffer_size > 0);
  ut_a(buffer + buffer_size <= log.buf + log.buf_size);
}

}
