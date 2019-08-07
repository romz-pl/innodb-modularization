#include <innodb/log_redo/srv_thread_concurrency.h>

/** The following controls how many threads we let inside InnoDB concurrently:
threads waiting for locks are not counted into the number because otherwise
we could get a deadlock. Value of 0 will disable the concurrency check. */

ulong srv_thread_concurrency = 0;
