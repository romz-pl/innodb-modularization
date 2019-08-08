#include <innodb/log_redo/srv_log_write_max_size.h>

#include <innodb/log_redo/flags.h>

/** When log writer follows links in the log recent written buffer,
it stops when it has reached at least that many bytes to write,
limiting how many bytes can be written in single call. */
ulong srv_log_write_max_size = INNODB_LOG_WRITE_MAX_SIZE_DEFAULT;


