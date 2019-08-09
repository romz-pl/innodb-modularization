#include <innodb/log_write/srv_log_write_notifier_spin_delay.h>

#include <innodb/log_types/flags.h>

/** Number of spin iterations, for which log write notifier thread is waiting
for advanced flushed_to_disk_lsn without sleeping. */
ulong srv_log_write_notifier_spin_delay =
    INNODB_LOG_WRITE_NOTIFIER_SPIN_DELAY_DEFAULT;
