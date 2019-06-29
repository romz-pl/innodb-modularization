#pragma once

#include <innodb/univ/univ.h>
#include <innodb/sync_event/os_event_t.h>

#ifndef UNIV_HOTBACKUP

/**
Waits for an event object until it is in the signaled state.

Typically, if the event has been signalled after the os_event_reset()
we'll return immediately because event->is_set == TRUE.
There are, however, situations (e.g.: sync_array code) where we may
lose this information. For example:

thread A calls os_event_reset()
thread B calls os_event_set()   [event->is_set == TRUE]
thread C calls os_event_reset() [event->is_set == FALSE]
thread A calls os_event_wait()  [infinite wait!]
thread C calls os_event_wait()  [infinite wait!]

Where such a scenario is possible, to avoid infinite wait, the
value returned by os_event_reset() should be passed in as
reset_sig_count. */
void os_event_wait_low(os_event_t event,         /*!< in/out: event to wait */
                       int64_t reset_sig_count); /*!< in: zero or the value
                                                returned by previous call of
                                                os_event_reset(). */

#endif

