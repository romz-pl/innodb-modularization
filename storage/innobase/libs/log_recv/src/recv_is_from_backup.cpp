#include <innodb/log_recv/recv_is_from_backup.h>

/** true when recovering from a backed up redo log file */
bool recv_is_from_backup = false;
