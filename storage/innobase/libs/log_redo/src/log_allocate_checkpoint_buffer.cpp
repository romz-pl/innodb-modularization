#include <innodb/log_redo/log_allocate_checkpoint_buffer.h>

#include <innodb/log_types/log_t.h>
#include <innodb/disk/flags.h>

void log_allocate_checkpoint_buffer(log_t &log) {
  log.checkpoint_buf.create(OS_FILE_LOG_BLOCK_SIZE);
}
