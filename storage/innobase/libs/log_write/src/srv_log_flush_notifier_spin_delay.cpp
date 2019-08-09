#include <innodb/log_write/srv_log_flush_notifier_spin_delay.h>

#include <innodb/log_types/flags.h>

/** Number of spin iterations, for which log flush notifier thread is waiting
for advanced flushed_to_disk_lsn without sleeping. */
ulong srv_log_flush_notifier_spin_delay =
    INNODB_LOG_FLUSH_NOTIFIER_SPIN_DELAY_DEFAULT;
