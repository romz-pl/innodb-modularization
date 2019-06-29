#pragma once

#include <innodb/univ/univ.h>
#include <innodb/sync_event/os_event_t.h>

#ifndef UNIV_HOTBACKUP

/**
Frees an event object. */
void os_event_destroy(os_event_t &event); /*!< in/own: event to free */

#endif
