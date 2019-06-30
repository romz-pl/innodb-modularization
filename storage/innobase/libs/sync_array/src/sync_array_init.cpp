#include <innodb/sync_array/sync_array_init.h>

#include <innodb/sync_array/sync_wait_array.h>
#include <innodb/sync_array/srv_sync_array_size.h>
#include <innodb/sync_array/sync_array_size.h>
#include <innodb/allocator/UT_NEW_ARRAY_NOKEY.h>

/** Create the primary system wait array(s), they are protected by an OS mutex
 */
void sync_array_init(ulint n_threads) /*!< in: Number of slots to
                                      create in all arrays */
{
  ut_a(sync_wait_array == NULL);
  ut_a(srv_sync_array_size > 0);
  ut_a(n_threads > 0);

  sync_array_size = srv_sync_array_size;

  sync_wait_array = UT_NEW_ARRAY_NOKEY(sync_array_t *, sync_array_size);

  ulint n_slots = 1 + (n_threads - 1) / sync_array_size;

  for (ulint i = 0; i < sync_array_size; ++i) {
    sync_wait_array[i] = UT_NEW_NOKEY(sync_array_t(n_slots));
  }
}
