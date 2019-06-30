#include <innodb/sync_array/sync_array_get_nth_cell.h>

/** Gets the nth cell in array.
 @return cell */
sync_cell_t *sync_array_get_nth_cell(
    sync_array_t *arr, /*!< in: sync array */
    ulint n)           /*!< in: index */
{
  ut_a(n < arr->n_cells);

  return (&arr->cells[n]);
}
