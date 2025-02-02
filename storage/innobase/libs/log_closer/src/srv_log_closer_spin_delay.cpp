#include <innodb/log_closer/srv_log_closer_spin_delay.h>

#include <innodb/log_types/flags.h>

/** Number of spin iterations, for which log closerr thread is waiting
for a reachable untraversed link in recent_closed. */
ulong srv_log_closer_spin_delay = INNODB_LOG_CLOSER_SPIN_DELAY_DEFAULT;
