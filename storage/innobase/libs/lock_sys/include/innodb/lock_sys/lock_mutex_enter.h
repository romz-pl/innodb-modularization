#pragma once

#include <innodb/univ/univ.h>

#include <innodb/lock_sys/lock_sys.h>
#include <innodb/sync_mutex/mutex_enter.h>

/** Acquire the lock_sys->mutex. */
#define lock_mutex_enter()         \
  do {                             \
    mutex_enter(&lock_sys->mutex); \
  } while (0)
