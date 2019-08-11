#include <innodb/log_wait/srv_log_wait_for_write_timeout.h>

#include <innodb/log_types/flags.h>

/** Timeout used when waiting for redo write (microseconds). */
ulong srv_log_wait_for_write_timeout =
    INNODB_LOG_WAIT_FOR_WRITE_TIMEOUT_DEFAULT;
