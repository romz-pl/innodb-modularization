#pragma once

#include <innodb/univ/univ.h>
#include <innodb/sync_event/os_event_t.h>

#ifndef UNIV_HOTBACKUP

/**
Sets an event semaphore to the signaled state: lets waiting threads
proceed. */
void os_event_set(os_event_t event); /*!< in/out: event to set */

#endif
