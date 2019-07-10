#include <innodb/ioasync/os_aio_validate.h>

#include <innodb/ioasync/AIO.h>

/** Validates the consistency the aio system.
@return true if ok */
bool os_aio_validate() {
  /* The methods countds and validates, we ignore the count. */
  AIO::total_pending_io_count();

  return (true);
}
