#pragma once

#include <innodb/univ/univ.h>
#include <innodb/sync_event/os_event_t.h>

#ifndef UNIV_HOTBACKUP

/**
Check if the event is set.
@return true if set */
bool os_event_is_set(const os_event_t event); /*!< in: event to set */

#endif
