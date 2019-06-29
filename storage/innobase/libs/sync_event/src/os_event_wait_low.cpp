#include <innodb/sync_event/os_event_wait_low.h>
#include <innodb/sync_event/os_event.h>

/**
Waits for an event object until it is in the signaled state.

Where such a scenario is possible, to avoid infinite wait, the
value returned by os_event_reset() should be passed in as
reset_sig_count. */
void os_event_wait_low(os_event_t event,        /*!< in: event to wait */
                       int64_t reset_sig_count) /*!< in: zero or the value
                                                returned by previous call of
                                                os_event_reset(). */
{
  event->wait_low(reset_sig_count);
}
