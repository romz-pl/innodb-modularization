#pragma once

#include <innodb/univ/univ.h>
#include <innodb/sync_event/os_event_t.h>

#ifndef UNIV_HOTBACKUP

/**
Waits for an event object until it is in the signaled state or
a timeout is exceeded. In Unix the timeout is always infinite.
@return 0 if success, OS_SYNC_TIME_EXCEEDED if timeout was exceeded */
ulint os_event_wait_time_low(
    os_event_t event,         /*!< in/out: event to wait */
    ulint time_in_usec,       /*!< in: timeout in
                              microseconds, or
                              OS_SYNC_INFINITE_TIME */
    int64_t reset_sig_count); /*!< in: zero or the value
                              returned by previous call of
                              os_event_reset(). */

#endif
