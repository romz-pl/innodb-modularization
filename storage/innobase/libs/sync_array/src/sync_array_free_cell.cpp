#include <innodb/sync_array/sync_array_free_cell.h>

#include <innodb/sync_array/sync_array_enter.h>
#include <innodb/sync_array/sync_array_exit.h>

/** Frees the cell. NOTE! sync_array_wait_event frees the cell
 automatically! */
void sync_array_free_cell(
    sync_array_t *arr,  /*!< in: wait array */
    sync_cell_t *&cell) /*!< in/out: the cell in the array */
{
  sync_array_enter(arr);

  ut_a(cell->latch.mutex != NULL);

  cell->waiting = false;
  cell->signal_count = 0;
  cell->latch.mutex = NULL;

  /* Setup the list of free slots in the array */
  cell->line = arr->first_free_slot;

  arr->first_free_slot = cell - arr->cells;

  ut_a(arr->n_reserved > 0);
  arr->n_reserved--;

  if (arr->next_free_slot > arr->n_cells / 2 && arr->n_reserved == 0) {
#ifdef UNIV_DEBUG
    for (ulint i = 0; i < arr->next_free_slot; ++i) {
      cell = sync_array_get_nth_cell(arr, i);

      ut_ad(!cell->waiting);
      ut_ad(cell->latch.mutex == 0);
      ut_ad(cell->signal_count == 0);
    }
#endif /* UNIV_DEBUG */
    arr->next_free_slot = 0;
    arr->first_free_slot = ULINT_UNDEFINED;
  }
  sync_array_exit(arr);

  cell = 0;
}
