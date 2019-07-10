#include <innodb/ioasync/srv_set_io_thread_op_info.h>

#include <innodb/ioasync/srv_io_thread_op_info.h>
#include <innodb/ioasync/SRV_MAX_N_IO_THREADS.h>
#include <innodb/assert/assert.h>

/** Sets the info describing an i/o thread current state. */
void srv_set_io_thread_op_info(
    ulint i,         /*!< in: the 'segment' of the i/o thread */
    const char *str) /*!< in: constant char string describing the
                     state */
{
  ut_a(i < SRV_MAX_N_IO_THREADS);

  srv_io_thread_op_info[i] = str;
}
