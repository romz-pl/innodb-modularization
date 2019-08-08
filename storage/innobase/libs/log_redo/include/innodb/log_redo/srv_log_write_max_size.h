#pragma once

#include <innodb/univ/univ.h>


/** When log writer follows links in the log recent written buffer,
it stops when it has reached at least that many bytes to write,
limiting how many bytes can be written in single call. */
extern ulong srv_log_write_max_size;

