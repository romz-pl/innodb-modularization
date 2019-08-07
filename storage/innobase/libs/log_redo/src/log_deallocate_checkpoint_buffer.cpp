#include <innodb/log_redo/log_deallocate_checkpoint_buffer.h>

#include <innodb/log_types/log_t.h>

void log_deallocate_checkpoint_buffer(log_t &log) {
  log.checkpoint_buf.destroy();
}
