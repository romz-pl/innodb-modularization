#include <innodb/sync_array/sync_arr_wake_threads_if_sema_free.h>

#include <innodb/sync_array/sync_array_size.h>
#include <innodb/sync_array/sync_wait_array.h>
#include <innodb/sync_array/sync_array_wake_threads_if_sema_free_low.h>

/** If the wakeup algorithm does not work perfectly at semaphore relases,
 this function will do the waking (see the comment in mutex_exit). This
 function should be called about every 1 second in the server.

 Note that there's a race condition between this thread and mutex_exit
 changing the lock_word and calling signal_object, so sometimes this finds
 threads to wake up even when nothing has gone wrong. */
void sync_arr_wake_threads_if_sema_free(void) {
  for (ulint i = 0; i < sync_array_size; ++i) {
    sync_array_wake_threads_if_sema_free_low(sync_wait_array[i]);
  }
}
