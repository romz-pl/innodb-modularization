#include <innodb/log_write/srv_log_wait_for_flush_timeout.h>

#include <innodb/log_types/flags.h>

/** Timeout used when waiting for redo flush (microseconds). */
ulong srv_log_wait_for_flush_timeout =
    INNODB_LOG_WAIT_FOR_FLUSH_TIMEOUT_DEFAULT;
