#include <innodb/log_files/log_deallocate_file_header_buffers.h>

#include <innodb/log_types/log_t.h>

void log_deallocate_file_header_buffers(log_t &log) {
  ut_a(log.n_files > 0);
  ut_a(log.file_header_bufs != nullptr);

  UT_DELETE_ARRAY(log.file_header_bufs);
  log.file_header_bufs = nullptr;
}
