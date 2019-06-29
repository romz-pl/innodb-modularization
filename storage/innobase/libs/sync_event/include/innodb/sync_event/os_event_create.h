#pragma once

#include <innodb/univ/univ.h>
#include <innodb/sync_event/os_event_t.h>

#ifndef UNIV_HOTBACKUP

/**
Creates an event semaphore, i.e., a semaphore which may just have two states:
signaled and nonsignaled. The created event is manual reset: it must be reset
explicitly by calling os_event_reset().
@return	the event handle */
os_event_t os_event_create(
    const char *name); /*!< in: the name of the event, if NULL
                       the event is created without a name */

#endif
