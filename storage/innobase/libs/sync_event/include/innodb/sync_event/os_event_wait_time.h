#pragma once

#include <innodb/univ/univ.h>
#include <innodb/sync_event/os_event_wait_time_low.h>

#ifndef UNIV_HOTBACKUP

/** Blocking timed wait on an event.
@param e - event to wait on.
@param t - timeout in microseconds */
#define os_event_wait_time(e, t) os_event_wait_time_low((e), (t), 0)

#endif
