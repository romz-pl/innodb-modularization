#pragma once

#include <innodb/univ/univ.h>
#include <innodb/sync_event/os_event_wait_low.h>

#ifndef UNIV_HOTBACKUP

/** Blocking infinite wait on an event, until signealled.
@param e - event to wait on. */
#define os_event_wait(e) os_event_wait_low((e), 0)

#endif
