#include <innodb/ioasync/os_aio_print.h>

#include <innodb/formatting/formatting.h>
#include <innodb/io/os_bytes_read_since_printout.h>
#include <innodb/io/os_n_file_reads.h>
#include <innodb/io/os_n_file_writes.h>
#include <innodb/io/os_n_fsyncs.h>
#include <innodb/io/os_n_pending_reads.h>
#include <innodb/io/os_n_pending_writes.h>
#include <innodb/ioasync/AIO.h>
#include <innodb/ioasync/os_aio_segment_wait_events.h>
#include <innodb/ioasync/os_last_printout.h>
#include <innodb/ioasync/srv_io_thread_function.h>
#include <innodb/ioasync/srv_io_thread_op_info.h>
#include <innodb/sync_event/os_event_is_set.h>
#include <innodb/time/ut_time.h>

extern ulint fil_n_pending_tablespace_flushes;
extern ulint srv_n_file_io_threads;
extern ulint fil_n_pending_log_flushes;

/** Prints info of the aio arrays.
@param[in,out]	file		file where to print */
void os_aio_print(FILE *file) {
  time_t current_time;
  double time_elapsed;
  double avg_bytes_read;

#ifndef UNIV_HOTBACKUP
  for (ulint i = 0; i < srv_n_file_io_threads; ++i) {
    fprintf(file, "I/O thread %lu state: %s (%s)", (ulong)i,
            srv_io_thread_op_info[i], srv_io_thread_function[i]);

#ifndef _WIN32
    if (os_event_is_set(os_aio_segment_wait_events[i])) {
      fprintf(file, " ev set");
    }
#endif /* _WIN32 */

    fprintf(file, "\n");
  }
#endif /* !UNIV_HOTBACKUP */

  fputs("Pending normal aio reads:", file);

  AIO::print_all(file);

  putc('\n', file);
  current_time = ut_time();
  time_elapsed = 0.001 + difftime(current_time, os_last_printout);

  fprintf(file,
          "Pending flushes (fsync) log: " ULINTPF
          "; "
          "buffer pool: " ULINTPF "\n" ULINTPF " OS file reads, " ULINTPF
          " OS file writes, " ULINTPF " OS fsyncs\n",
          fil_n_pending_log_flushes, fil_n_pending_tablespace_flushes,
          os_n_file_reads, os_n_file_writes, os_n_fsyncs);

  if (os_n_pending_writes != 0 || os_n_pending_reads != 0) {
    fprintf(file, ULINTPF " pending preads, " ULINTPF " pending pwrites\n",
            os_n_pending_reads, os_n_pending_writes);
  }

  if (os_n_file_reads == os_n_file_reads_old) {
    avg_bytes_read = 0.0;
  } else {
    avg_bytes_read = (double)os_bytes_read_since_printout /
                     (os_n_file_reads - os_n_file_reads_old);
  }

  fprintf(file,
          "%.2f reads/s, %lu avg bytes/read,"
          " %.2f writes/s, %.2f fsyncs/s\n",
          (os_n_file_reads - os_n_file_reads_old) / time_elapsed,
          (ulong)avg_bytes_read,
          (os_n_file_writes - os_n_file_writes_old) / time_elapsed,
          (os_n_fsyncs - os_n_fsyncs_old) / time_elapsed);

  os_n_file_reads_old = os_n_file_reads;
  os_n_file_writes_old = os_n_file_writes;
  os_n_fsyncs_old = os_n_fsyncs;
  os_bytes_read_since_printout = 0;

  os_last_printout = current_time;
}
