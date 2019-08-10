#include <innodb/log_buffer/log_buffer_s_lock_enter.h>

#include <innodb/log_types/log_t.h>

size_t log_buffer_s_lock_enter(log_t &log) {
    return (log.sn_lock.s_lock());
}
