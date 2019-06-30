#include <innodb/sync_array/sync_array_print.h>

#include <innodb/sync_array/sync_array_size.h>
#include <innodb/sync_array/sync_wait_array.h>
#include <innodb/sync_array/sync_array_print_info.h>
#include <innodb/sync_array/sg_count.h>
#include <innodb/formatting/formatting.h>

/** Print info about the sync array(s). */
void sync_array_print(FILE *file) /*!< in/out: Print to this stream */
{
  for (ulint i = 0; i < sync_array_size; ++i) {
    sync_array_print_info(file, sync_wait_array[i]);
  }

  fprintf(file, "OS WAIT ARRAY INFO: signal count " ULINTPF "\n", sg_count);
}
