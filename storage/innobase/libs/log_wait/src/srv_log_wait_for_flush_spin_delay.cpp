#include <innodb/log_wait/srv_log_wait_for_flush_spin_delay.h>

#include <innodb/log_types/flags.h>

/** Number of spin iterations, when spinning and waiting for log flushed. */
ulong srv_log_wait_for_flush_spin_delay =
    INNODB_LOG_WAIT_FOR_FLUSH_SPIN_DELAY_DEFAULT;
