#include <innodb/sync_array/sync_array_print_long_waits_low.h>

#include <innodb/sync_array/sync_array_get_nth_cell.h>
#include <innodb/sync_array/sync_array_cell_print.h>
#include <innodb/logger/warn.h>

extern ulint srv_fatal_semaphore_wait_threshold;

// copied from "srv0srv.h"
#define SRV_SEMAPHORE_WAIT_EXTENSION 7200

/** Prints warnings of long semaphore waits to stderr.
 @return true if fatal semaphore wait threshold was exceeded */
bool sync_array_print_long_waits_low(
    sync_array_t *arr,      /*!< in: sync array instance */
    os_thread_id_t *waiter, /*!< out: longest waiting thread */
    const void **sema,      /*!< out: longest-waited-for semaphore */
    ibool *noticed)         /*!< out: TRUE if long wait noticed */
{
  ulint fatal_timeout = srv_fatal_semaphore_wait_threshold;
  ibool fatal = FALSE;
  double longest_diff = 0;

  /* For huge tables, skip the check during CHECK TABLE etc... */
  if (fatal_timeout > SRV_SEMAPHORE_WAIT_EXTENSION) {
    return (false);
  }

#ifdef UNIV_DEBUG_VALGRIND
    /* Increase the timeouts if running under valgrind because it executes
    extremely slowly. UNIV_DEBUG_VALGRIND does not necessary mean that
    we are running under valgrind but we have no better way to tell.
    See Bug#58432 innodb.innodb_bug56143 fails under valgrind
    for an example */
#define SYNC_ARRAY_TIMEOUT 2400
  fatal_timeout *= 10;
#else
#define SYNC_ARRAY_TIMEOUT 240
#endif

  for (ulint i = 0; i < arr->n_cells; i++) {
    sync_cell_t *cell;
    void *latch;

    cell = sync_array_get_nth_cell(arr, i);

    latch = cell->latch.mutex;

    if (latch == NULL || !cell->waiting) {
      continue;
    }

    double diff = difftime(time(NULL), cell->reservation_time);

    if (diff > SYNC_ARRAY_TIMEOUT) {
#ifdef UNIV_NO_ERR_MSGS
      ib::warn()
#else
      ib::warn(ER_IB_MSG_1160)
#endif /* UNIV_NO_ERR_MSGS */
          << "A long semaphore wait:";

      sync_array_cell_print(stderr, cell);
      *noticed = TRUE;
    }

    if (diff > fatal_timeout) {
      fatal = TRUE;
    }

    if (diff > longest_diff) {
      longest_diff = diff;
      *sema = latch;
      *waiter = cell->thread_id;
    }
  }

#undef SYNC_ARRAY_TIMEOUT

  return (fatal);
}
