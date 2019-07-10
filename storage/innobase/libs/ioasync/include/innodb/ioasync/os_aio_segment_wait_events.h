#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_event/os_event_t.h>

/** Array of events used in simulated AIO */
extern os_event_t *os_aio_segment_wait_events;
