#include <innodb/ioasync/os_aio_refresh_stats.h>

#include <innodb/io/os_bytes_read_since_printout.h>
#include <innodb/io/os_n_file_reads.h>
#include <innodb/io/os_n_file_writes.h>
#include <innodb/io/os_n_fsyncs.h>
#include <innodb/io/os_n_fsyncs.h>
#include <innodb/ioasync/os_last_printout.h>
#include <innodb/time/ut_time.h>

/** Refreshes the statistics used to print per-second averages. */
void os_aio_refresh_stats() {
  os_n_fsyncs_old = os_n_fsyncs;

  os_bytes_read_since_printout = 0;

  os_n_file_reads_old = os_n_file_reads;

  os_n_file_writes_old = os_n_file_writes;

  os_n_fsyncs_old = os_n_fsyncs;

  os_bytes_read_since_printout = 0;

  os_last_printout = ut_time();
}
