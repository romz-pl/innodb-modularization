#include <innodb/log_sys/log_deallocate_write_events.h>

#include <innodb/log_types/log_t.h>

void log_deallocate_write_events(log_t &log) {
  ut_a(log.write_events != nullptr);

  for (size_t i = 0; i < log.write_events_size; ++i) {
    os_event_destroy(log.write_events[i]);
  }

  UT_DELETE_ARRAY(log.write_events);
  log.write_events = nullptr;
}
