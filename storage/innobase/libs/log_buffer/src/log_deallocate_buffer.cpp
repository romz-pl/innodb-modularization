#include <innodb/log_buffer/log_deallocate_buffer.h>

#include <innodb/log_types/log_t.h>

void log_deallocate_buffer(log_t &log) {
    log.buf.destroy();
}
