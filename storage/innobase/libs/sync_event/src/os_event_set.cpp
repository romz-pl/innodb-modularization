#include <innodb/sync_event/os_event_set.h>
#include <innodb/sync_event/os_event.h>

/**
Sets an event semaphore to the signaled state: lets waiting threads
proceed. */
void os_event_set(os_event_t event) /*!< in/out: event to set */
{
  event->set();
}
