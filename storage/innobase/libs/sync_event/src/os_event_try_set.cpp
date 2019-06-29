#include <innodb/sync_event/os_event_try_set.h>
#include <innodb/sync_event/os_event.h>

bool os_event_try_set(os_event_t event) {
    return (event->try_set());
}
