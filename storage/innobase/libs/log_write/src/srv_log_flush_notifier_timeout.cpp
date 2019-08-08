#include <innodb/log_write/srv_log_flush_notifier_timeout.h>

#include <innodb/log_redo/flags.h>

/** Initial timeout used to wait on flush_notifier_event. */
ulong srv_log_flush_notifier_timeout =
    INNODB_LOG_FLUSH_NOTIFIER_TIMEOUT_DEFAULT;
