#include <innodb/log_flush/log_deallocate_flush_events.h>

#include <innodb/log_types/log_t.h>

void log_deallocate_flush_events(log_t &log) {
  ut_a(log.flush_events != nullptr);

  for (size_t i = 0; i < log.flush_events_size; ++i) {
    os_event_destroy(log.flush_events[i]);
  }

  UT_DELETE_ARRAY(log.flush_events);
  log.flush_events = nullptr;
}
