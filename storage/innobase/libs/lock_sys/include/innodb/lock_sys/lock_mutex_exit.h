#pragma once

#include <innodb/univ/univ.h>

#include <innodb/lock_sys/lock_sys.h>

/** Release the lock_sys->mutex. */
#define lock_mutex_exit()   \
  do {                      \
    lock_sys->mutex.exit(); \
  } while (0)
