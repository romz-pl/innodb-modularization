#include <innodb/log_redo/srv_log_flush_events.h>

#include <innodb/log_types/flags.h>

/** Number of events used for notifications about redo flush. */
ulong srv_log_flush_events = INNODB_LOG_EVENTS_DEFAULT;
