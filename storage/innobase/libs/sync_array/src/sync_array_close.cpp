#include <innodb/sync_array/sync_array_close.h>

#include <innodb/sync_array/sync_array_size.h>
#include <innodb/sync_array/sync_array_free.h>
#include <innodb/sync_array/sync_wait_array.h>
#include <innodb/allocator/UT_DELETE_ARRAY.h>

/** Close sync array wait sub-system. */
void sync_array_close(void) {
  for (ulint i = 0; i < sync_array_size; ++i) {
    sync_array_free(sync_wait_array[i]);
  }

  UT_DELETE_ARRAY(sync_wait_array);
  sync_wait_array = NULL;
}
