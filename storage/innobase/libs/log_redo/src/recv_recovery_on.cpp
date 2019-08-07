#include <innodb/log_redo/recv_recovery_on.h>

/** true when applying redo log records during crash recovery; false
otherwise.  Note that this is false while a background thread is
rolling back incomplete transactions. */
volatile bool recv_recovery_on;
