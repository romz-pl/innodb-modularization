#include <innodb/log_redo/log_allocate_flush_events.h>

#include <innodb/log_types/log_t.h>
#include <innodb/log_types/flags.h>
#include <innodb/log_redo/srv_log_flush_events.h>
#include <innodb/assert/assert.h>
#include <innodb/sync_event/os_event_create.h>

void log_allocate_flush_events(log_t &log) {
  const size_t n = srv_log_flush_events;

  ut_a(log.flush_events == nullptr);
  ut_a(n >= 1);
  ut_a((n & (n - 1)) == 0);

  log.flush_events_size = n;
  log.flush_events = UT_NEW_ARRAY_NOKEY(os_event_t, n);

  for (size_t i = 0; i < log.flush_events_size; ++i) {
    log.flush_events[i] = os_event_create("log_flush_event");
  }
}
