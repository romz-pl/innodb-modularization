#pragma once

#include <innodb/univ/univ.h>

/** Space for log buffer, expressed in bytes. Note, that log buffer
will use only the largest power of two, which is not greater than
the assigned space. */
extern ulong srv_log_buffer_size;
