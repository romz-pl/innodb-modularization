#include <innodb/log_write/srv_log_recent_written_size.h>

#include <innodb/log_types/flags.h>

/** Number of slots in a small buffer, which is used to allow concurrent
writes to log buffer. The slots are addressed by LSN values modulo number
of the slots. */
ulong srv_log_recent_written_size = INNODB_LOG_RECENT_WRITTEN_SIZE_DEFAULT;
