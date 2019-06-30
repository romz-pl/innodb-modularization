#include <innodb/sync_array/sync_array_get.h>

#include <innodb/sync_array/sync_array_size.h>
#include <innodb/sync_array/sync_wait_array.h>
#include <innodb/counter/counter.h>

/** Get an instance of the sync wait array.
 @return an instance of the sync wait array. */
sync_array_t *sync_array_get() {
  if (sync_array_size <= 1) {
    return (sync_wait_array[0]);
  }

  return (
      sync_wait_array[default_indexer_t<>::get_rnd_index() % sync_array_size]);
}

