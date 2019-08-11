#pragma once

#include <innodb/univ/univ.h>

/** Number of slots in a small buffer, which is used to break requirement
for total order of dirty pages, when they are added to flush lists.
The slots are addressed by LSN values modulo number of the slots. */
extern ulong srv_log_recent_closed_size;
