#include <innodb/ioasync/srv_io_thread_op_info.h>

#include <innodb/ioasync/SRV_MAX_N_IO_THREADS.h>

/** Array of English strings describing the current state of an
i/o handler thread */
const char *srv_io_thread_op_info[SRV_MAX_N_IO_THREADS];
