#include <innodb/sync_event/os_event_destroy.h>
#include <innodb/allocator/UT_DELETE.h>
#include <innodb/sync_event/os_event.h>

/**
Frees an event object. */
void os_event_destroy(os_event_t &event) /*!< in/own: event to free */

{
  if (event != NULL) {
    UT_DELETE(event);
    event = NULL;
  }
}
