#include <innodb/log_closer/srv_log_recent_closed_size.h>

#include <innodb/log_types/flags.h>

/** Number of slots in a small buffer, which is used to break requirement
for total order of dirty pages, when they are added to flush lists.
The slots are addressed by LSN values modulo number of the slots. */
ulong srv_log_recent_closed_size = INNODB_LOG_RECENT_CLOSED_SIZE_DEFAULT;
