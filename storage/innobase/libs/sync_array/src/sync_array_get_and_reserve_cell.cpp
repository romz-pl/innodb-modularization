#include <innodb/sync_array/sync_array_get_and_reserve_cell.h>

#include <innodb/sync_array/sync_array_size.h>
#include <innodb/sync_array/sync_array_get.h>
#include <innodb/sync_array/sync_array_reserve_cell.h>

/** Get an instance of the sync wait array and reserve a wait array cell
 in the instance for waiting for an object. The event of the cell is
 reset to nonsignalled state.
 If reserving cell of the instance fails, try to get another new
 instance until we can reserve an empty cell of it.
 @return the sync array reserved, never NULL. */
sync_array_t *sync_array_get_and_reserve_cell(
    void *object,       /*!< in: pointer to the object to wait for */
    ulint type,         /*!< in: lock request type */
    const char *file,   /*!< in: file where requested */
    ulint line,         /*!< in: line where requested */
    sync_cell_t **cell) /*!< out: the cell reserved, never NULL */
{
  sync_array_t *sync_arr = NULL;

  *cell = NULL;
  for (ulint i = 0; i < sync_array_size && *cell == NULL; ++i) {
    /* Although the sync_array is get in a random way currently,
    we still try at most sync_array_size times, in case any
    of the sync_array we get is full */
    sync_arr = sync_array_get();
    *cell = sync_array_reserve_cell(sync_arr, object, type, file, line);
  }

  /* This won't be true every time, for the loop above may execute
  more than srv_sync_array_size times to reserve a cell.
  But an assertion here makes the code more solid. */
  ut_a(*cell != NULL);

  return (sync_arr);
}
