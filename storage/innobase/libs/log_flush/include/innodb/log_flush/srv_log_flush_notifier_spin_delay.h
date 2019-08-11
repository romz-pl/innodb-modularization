#pragma once

#include <innodb/univ/univ.h>

/** Number of spin iterations, for which log flush notifier thread is waiting
for advanced flushed_to_disk_lsn without sleeping. */
extern ulong srv_log_flush_notifier_spin_delay;
