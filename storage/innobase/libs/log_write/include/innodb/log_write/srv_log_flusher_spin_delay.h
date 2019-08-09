#pragma once

#include <innodb/univ/univ.h>

/** Number of spin iterations, for which log flusher thread is waiting
for new data to flush, without sleeping. */
extern ulong srv_log_flusher_spin_delay;
