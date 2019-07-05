#include <innodb/ioasync/os_aio_n_segments.h>

/** Number of asynchronous I/O segments.  Set by os_aio_init(). */
ulint os_aio_n_segments = ULINT_UNDEFINED;
