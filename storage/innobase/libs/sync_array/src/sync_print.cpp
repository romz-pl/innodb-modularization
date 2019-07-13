#include <innodb/sync_array/sync_print.h>

#include <innodb/sync_array/sync_array_print.h>
#include <innodb/sync_rw/sync_print_wait_info.h>

/**
Prints info of the sync system.
@param file - where to print */
void sync_print(FILE *file) {
#ifdef UNIV_DEBUG
  rw_lock_list_print_info(file);
#endif /* UNIV_DEBUG */

  sync_array_print(file);

  sync_print_wait_info(file);
}
