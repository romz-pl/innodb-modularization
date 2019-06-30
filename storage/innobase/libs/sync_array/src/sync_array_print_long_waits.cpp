#include <innodb/sync_array/sync_array_print_long_waits.h>

#include <innodb/sync_array/sync_array_size.h>
#include <innodb/sync_array/sync_wait_array.h>
#include <innodb/sync_array/sync_array_enter.h>
#include <innodb/sync_array/sync_array_print_long_waits_low.h>
#include <innodb/sync_array/sync_array_exit.h>

/** Set the lock system timeout event. */
void lock_set_timeout_event();

extern bool srv_print_innodb_monitor;

/** Number of pending read operations */
extern ulint os_n_pending_reads;

/** Number of pending write operations */
extern ulint os_n_pending_writes;

/** Prints warnings of long semaphore waits to stderr.
 @return true if fatal semaphore wait threshold was exceeded */
ibool sync_array_print_long_waits(
    os_thread_id_t *waiter, /*!< out: longest waiting thread */
    const void **sema)      /*!< out: longest-waited-for semaphore */
{
  ulint i;
  ibool fatal = FALSE;
  ibool noticed = FALSE;

  for (i = 0; i < sync_array_size; ++i) {
    sync_array_t *arr = sync_wait_array[i];

    sync_array_enter(arr);

    if (sync_array_print_long_waits_low(arr, waiter, sema, &noticed)) {
      fatal = TRUE;
    }

    sync_array_exit(arr);
  }

  if (noticed) {
    ibool old_val;

    fprintf(stderr,
            "InnoDB: ###### Starts InnoDB Monitor"
            " for 30 secs to print diagnostic info:\n");

    old_val = srv_print_innodb_monitor;

    /* If some crucial semaphore is reserved, then also the InnoDB
    Monitor can hang, and we do not get diagnostics. Since in
    many cases an InnoDB hang is caused by a pwrite() or a pread()
    call hanging inside the operating system, let us print right
    now the values of pending calls of these. */

    fprintf(stderr, "InnoDB: Pending preads %lu, pwrites %lu\n",
            (ulong)os_n_pending_reads, (ulong)os_n_pending_writes);

    srv_print_innodb_monitor = TRUE;

#ifndef UNIV_NO_ERR_MSGS
    lock_set_timeout_event();
#endif /* !UNIV_NO_ERR_MSGS */

    os_thread_sleep(30000000);

    srv_print_innodb_monitor = static_cast<bool>(old_val);
    fprintf(stderr,
            "InnoDB: ###### Diagnostic info printed"
            " to the standard error stream\n");
  }

  return (fatal);
}
