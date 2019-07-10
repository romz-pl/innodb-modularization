#include <innodb/ioasync/os_aio_simulated_put_read_threads_to_sleep.h>

/** This function can be called if one wants to post a batch of reads and
prefers an i/o-handler thread to handle them all at once later. You must
call os_aio_simulated_wake_handler_threads later to ensure the threads
are not left sleeping! */
void os_aio_simulated_put_read_threads_to_sleep() { /* No op on non Windows */
}
