#include <innodb/ioasync/srv_reset_io_thread_op_info.h>

#include <innodb/ioasync/srv_io_thread_op_info.h>
#include <innodb/ioasync/SRV_MAX_N_IO_THREADS.h>

/** Resets the info describing an i/o thread current state. */
void srv_reset_io_thread_op_info() {
  // for (ulint i = 0; i < UT_ARR_SIZE(srv_io_thread_op_info); ++i) {
  for (ulint i = 0; i < SRV_MAX_N_IO_THREADS; ++i) {
    srv_io_thread_op_info[i] = "not started yet";
  }
}
