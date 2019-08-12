#include <innodb/log_recv/recv_is_making_a_backup.h>

/** true When the redo log is being backed up */
bool recv_is_making_a_backup = false;
