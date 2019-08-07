#pragma once

#include <innodb/univ/univ.h>

/** Number of slots in a small buffer, which is used to allow concurrent
writes to log buffer. The slots are addressed by LSN values modulo number
of the slots. */
extern ulong srv_log_recent_written_size;

