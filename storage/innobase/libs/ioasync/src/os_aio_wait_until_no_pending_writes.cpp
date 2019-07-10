#include <innodb/ioasync/os_aio_wait_until_no_pending_writes.h>

#include <innodb/ioasync/AIO.h>

/** Waits until there are no pending writes in AIO::s_writes. There can
be other, synchronous, pending writes. */
void os_aio_wait_until_no_pending_writes() {
  AIO::wait_until_no_pending_writes();
}
