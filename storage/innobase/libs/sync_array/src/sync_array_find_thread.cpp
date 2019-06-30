#include <innodb/sync_array/sync_array_find_thread.h>

#ifdef UNIV_DEBUG

/** Looks for a cell with the given thread id.
 @return pointer to cell or NULL if not found */
static sync_cell_t *sync_array_find_thread(
    sync_array_t *arr,     /*!< in: wait array */
    os_thread_id_t thread) /*!< in: thread id */
{
  ulint i;

  for (i = 0; i < arr->n_cells; i++) {
    sync_cell_t *cell;

    cell = sync_array_get_nth_cell(arr, i);

    if (cell->latch.mutex != NULL && os_thread_eq(cell->thread_id, thread)) {
      return (cell); /* Found */
    }
  }

  return (NULL); /* Not found */
}

#endif
