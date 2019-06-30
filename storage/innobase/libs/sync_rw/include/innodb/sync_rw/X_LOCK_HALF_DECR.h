#pragma once

#include <innodb/univ/univ.h>

/* We decrement lock_word by X_LOCK_HALF_DECR for sx_lock. */
#define X_LOCK_HALF_DECR 0x10000000
