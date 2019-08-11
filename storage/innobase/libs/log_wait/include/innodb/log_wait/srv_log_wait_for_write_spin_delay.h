#pragma once

#include <innodb/univ/univ.h>

/** Number of spin iterations, when spinning and waiting for log buffer
written up to given LSN, before we fallback to loop with sleeps.
This is not used when user thread has to wait for log flushed to disk. */
extern ulong srv_log_wait_for_write_spin_delay;
