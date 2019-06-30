#pragma once

#include <innodb/univ/univ.h>

/* We decrement lock_word by X_LOCK_DECR for each x_lock. It is also the
start value for the lock_word, meaning that it limits the maximum number
of concurrent read locks before the rw_lock breaks. */
#define X_LOCK_DECR 0x20000000
