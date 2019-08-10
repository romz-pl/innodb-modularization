#include <innodb/log_files/log_allocate_file_header_buffers.h>

#include <innodb/log_types/log_t.h>

void log_allocate_file_header_buffers(log_t &log) {
  const uint32_t n_files = log.n_files;

  using Buf_ptr = aligned_array_pointer<byte, OS_FILE_LOG_BLOCK_SIZE>;

  log.file_header_bufs = UT_NEW_ARRAY_NOKEY(Buf_ptr, n_files);

  for (uint32_t i = 0; i < n_files; i++) {
    log.file_header_bufs[i].create(LOG_FILE_HDR_SIZE);
  }
}
