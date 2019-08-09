#include <innodb/log_write/srv_flushing_avg_loops.h>

/* Number of iterations over which adaptive flushing is averaged. */
ulong srv_flushing_avg_loops = 30;
