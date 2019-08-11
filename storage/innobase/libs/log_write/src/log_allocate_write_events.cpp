#include <innodb/log_write/log_allocate_write_events.h>

#include <innodb/log_types/log_t.h>
#include <innodb/log_types/flags.h>
#include <innodb/log_write/srv_log_write_events.h>
#include <innodb/assert/assert.h>

void log_allocate_write_events(log_t &log) {
  const size_t n = srv_log_write_events;

  ut_a(log.write_events == nullptr);
  ut_a(n >= 1);
  ut_a((n & (n - 1)) == 0);

  log.write_events_size = n;
  log.write_events = UT_NEW_ARRAY_NOKEY(os_event_t, n);

  for (size_t i = 0; i < log.write_events_size; ++i) {
    log.write_events[i] = os_event_create("log_write_event");
  }
}
