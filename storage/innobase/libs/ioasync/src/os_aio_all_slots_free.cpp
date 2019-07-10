#include <innodb/ioasync/os_aio_all_slots_free.h>

#include <innodb/ioasync/AIO.h>

/** Checks that all slots in the system have been freed, that is, there are
no pending io operations.
@return true if all free */
bool os_aio_all_slots_free() {
    return (AIO::total_pending_io_count() == 0);
}
