#include <innodb/sync_array/sync_array_validate.h>


#ifdef UNIV_DEBUG

/** Validates the integrity of the wait array. Checks
 that the number of reserved cells equals the count variable. */
void sync_array_validate(sync_array_t *arr) /*!< in: sync wait array */
{
  ulint count = 0;

  sync_array_enter(arr);

  for (ulint i = 0; i < arr->n_cells; i++) {
    const sync_cell_t *cell;

    cell = &arr->cells[i];

    if (cell->latch.mutex != NULL) {
      count++;
    }
  }

  ut_a(count == arr->n_reserved);

  sync_array_exit(arr);
}
#endif /* UNIV_DEBUG */
