#pragma once

#include <innodb/univ/univ.h>

struct log_t;

/** Stops all the log background threads. This can be called only,
when the threads are active. This should never be called concurrently.
This may not be called in read-only mode. Note that is is impossible
to start log background threads in such case.
@param[in,out]	log	redo log */
void log_stop_background_threads(log_t &log);
