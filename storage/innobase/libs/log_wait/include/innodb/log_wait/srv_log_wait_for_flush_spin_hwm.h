#pragma once

#include <innodb/univ/univ.h>

/** Maximum value of average log flush time for which spin-delay is used.
When flushing takes longer, user threads no longer spin when waiting for
flushed redo. Expressed in microseconds. */
extern ulong srv_log_wait_for_flush_spin_hwm;
