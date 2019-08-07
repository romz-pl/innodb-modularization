#pragma once

#include <innodb/univ/univ.h>

/** Size of block, used for writing ahead to avoid read-on-write. */
extern ulong srv_log_write_ahead_size;
