#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_event/os_event_t.h>

/** Archiver thread event to signal that data is available */
extern os_event_t page_archiver_thread_event;
