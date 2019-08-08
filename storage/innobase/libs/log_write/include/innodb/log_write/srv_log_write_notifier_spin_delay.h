#pragma once

#include <innodb/univ/univ.h>

/** Number of spin iterations, for which log write notifier thread is waiting
for advanced writeed_to_disk_lsn without sleeping. */
extern ulong srv_log_write_notifier_spin_delay;
