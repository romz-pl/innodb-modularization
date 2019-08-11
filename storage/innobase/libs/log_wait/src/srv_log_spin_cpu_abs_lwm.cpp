#include <innodb/log_wait/srv_log_spin_cpu_abs_lwm.h>

/** Minimum absolute value of cpu time for which spin-delay is used. */
uint srv_log_spin_cpu_abs_lwm;
