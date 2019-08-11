#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

/** Any database operation should call this when it has modified more than
about 4 pages. NOTE that this function may only be called when the thread
owns no synchronization objects except the dictionary mutex.

Checks if current log.sn exceeds log.sn_limit_for_start, in which case waits.
This is supposed to guarantee that we would not run out of space in the log
files when holding latches of some dirty pages (which could end up in
a deadlock, because flush of the latched dirty pages could be required
to reclaim the space and it is impossible to flush latched pages). */
void log_free_check();

#endif
