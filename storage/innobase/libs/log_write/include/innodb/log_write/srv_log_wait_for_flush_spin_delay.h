#pragma once

#include <innodb/univ/univ.h>

/** Number of spin iterations, when spinning and waiting for log flushed. */
extern ulong srv_log_wait_for_flush_spin_delay;
