#pragma once

#include <innodb/univ/univ.h>

#include <innodb/lock_sys/lock_sys.h>

/** Release the lock_sys->wait_mutex. */
#define lock_wait_mutex_exit()   \
  do {                           \
    lock_sys->wait_mutex.exit(); \
  } while (0)
