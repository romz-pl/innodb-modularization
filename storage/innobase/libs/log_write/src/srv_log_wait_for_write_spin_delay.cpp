#include <innodb/log_write/srv_log_wait_for_write_spin_delay.h>

#include <innodb/log_redo/flags.h>

/** Number of spin iterations, when spinning and waiting for log buffer
written up to given LSN, before we fallback to loop with sleeps.
This is not used when user thread has to wait for log flushed to disk. */
ulong srv_log_wait_for_write_spin_delay =
    INNODB_LOG_WAIT_FOR_WRITE_SPIN_DELAY_DEFAULT;
