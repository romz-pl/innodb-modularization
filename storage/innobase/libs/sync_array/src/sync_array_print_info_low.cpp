#include <innodb/sync_array/sync_array_print_info_low.h>

#include <innodb/formatting/formatting.h>
#include <innodb/sync_array/sync_array_get_nth_cell.h>
#include <innodb/sync_array/sync_array_cell_print.h>

/** Prints info of the wait array. */
void sync_array_print_info_low(
    FILE *file,        /*!< in: file where to print */
    sync_array_t *arr) /*!< in: wait array */
{
  ulint i;
  ulint count = 0;

  fprintf(file, "OS WAIT ARRAY INFO: reservation count " ULINTPF "\n",
          arr->res_count);

  for (i = 0; count < arr->n_reserved; ++i) {
    sync_cell_t *cell;

    cell = sync_array_get_nth_cell(arr, i);

    if (cell->latch.mutex != 0) {
      count++;
      sync_array_cell_print(file, cell);
    }
  }
}
