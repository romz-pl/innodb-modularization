#pragma once

#include <innodb/univ/univ.h>

/** Number of spin iterations, for which log closerr thread is waiting
for a reachable untraversed link in recent_closed. */
extern ulong srv_log_closer_spin_delay;
