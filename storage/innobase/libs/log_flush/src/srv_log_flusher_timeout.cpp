#include <innodb/log_flush/srv_log_flusher_timeout.h>

#include <innodb/log_types/flags.h>

/** Initial timeout used to wait on flusher_event. */
ulong srv_log_flusher_timeout = INNODB_LOG_FLUSHER_TIMEOUT_DEFAULT;
