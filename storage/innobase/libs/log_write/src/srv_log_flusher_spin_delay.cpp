#include <innodb/log_write/srv_log_flusher_spin_delay.h>

#include <innodb/log_types/flags.h>

/** Number of spin iterations, for which log flusher thread is waiting
for new data to flush, without sleeping. */
ulong srv_log_flusher_spin_delay = INNODB_LOG_FLUSHER_SPIN_DELAY_DEFAULT;
