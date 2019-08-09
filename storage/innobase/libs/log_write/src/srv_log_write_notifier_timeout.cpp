#include <innodb/log_write/srv_log_write_notifier_timeout.h>

#include <innodb/log_types/flags.h>

/** Initial timeout used to wait on write_notifier_event. */
ulong srv_log_write_notifier_timeout =
    INNODB_LOG_WRITE_NOTIFIER_TIMEOUT_DEFAULT;
