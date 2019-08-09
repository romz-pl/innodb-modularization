#include <innodb/log_write/srv_log_writer_timeout.h>

#include <innodb/log_redo/flags.h>

/** Initial timeout used to wait on writer_event. */
ulong srv_log_writer_timeout = INNODB_LOG_WRITER_TIMEOUT_DEFAULT;
