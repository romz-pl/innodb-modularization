#include <innodb/sync_array/sync_array_print_info.h>

#include <innodb/sync_array/sync_array_enter.h>
#include <innodb/sync_array/sync_array_print_info_low.h>
#include <innodb/sync_array/sync_array_exit.h>

/** Prints info of the wait array. */
void sync_array_print_info(FILE *file, /*!< in: file where to print */
                                  sync_array_t *arr) /*!< in: wait array */
{
  sync_array_enter(arr);

  sync_array_print_info_low(file, arr);

  sync_array_exit(arr);
}
