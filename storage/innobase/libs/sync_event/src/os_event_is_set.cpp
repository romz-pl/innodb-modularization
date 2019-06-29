#include <innodb/sync_event/os_event_is_set.h>
#include <innodb/sync_event/os_event.h>

/**
Check if the event is set.
@return true if set */
bool os_event_is_set(const os_event_t event) /*!< in: event to test */
{
  return (event->is_set());
}
