#include <innodb/ioasync/os_aio_print_pending_io.h>

#ifdef UNIV_DEBUG

#include <innodb/ioasync/AIO.h>

/** Prints all pending IO
@param[in]	file		File where to print */
void os_aio_print_pending_io(FILE *file) {
    AIO::print_to_file(file);
}

#endif /* UNIV_DEBUG */

