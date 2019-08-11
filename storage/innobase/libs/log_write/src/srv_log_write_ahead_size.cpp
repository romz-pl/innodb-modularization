#include <innodb/log_write/srv_log_write_ahead_size.h>

/** Size of block, used for writing ahead to avoid read-on-write. */
ulong srv_log_write_ahead_size;
