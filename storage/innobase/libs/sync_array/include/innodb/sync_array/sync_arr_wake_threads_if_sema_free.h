#pragma once

#include <innodb/univ/univ.h>

/** If the wakeup algorithm does not work perfectly at semaphore relases,
 this function will do the waking (see the comment in mutex_exit). This
 function should be called about every 1 second in the server.

 Note that there's a race condition between this thread and mutex_exit
 changing the lock_word and calling signal_object, so sometimes this finds
 threads to wake up even when nothing has gone wrong. */
void sync_arr_wake_threads_if_sema_free(void);
