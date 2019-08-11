#include <innodb/log_closer/srv_log_closer_timeout.h>

#include <innodb/log_types/flags.h>

/** Initial sleep used in log closer after spin delay is finished. */
ulong srv_log_closer_timeout = INNODB_LOG_CLOSER_TIMEOUT_DEFAULT;
