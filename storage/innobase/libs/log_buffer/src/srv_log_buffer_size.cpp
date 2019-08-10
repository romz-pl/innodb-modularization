#include <innodb/log_buffer/srv_log_buffer_size.h>

/** Space for log buffer, expressed in bytes. Note, that log buffer
will use only the largest power of two, which is not greater than
the assigned space. */
ulong srv_log_buffer_size;
