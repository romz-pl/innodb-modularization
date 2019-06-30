#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_rw/rw_lock_t.h>
#include <innodb/sync_array/BlockWaitMutex.h>
#include <innodb/sync_array/WaitMutex.h>

/** The latch types that use the sync array. */
union sync_object_t {
  /** RW lock instance */
  rw_lock_t *lock;

  /** Mutex instance */
  WaitMutex *mutex;

  /** Block mutex instance */
  BlockWaitMutex *bpmutex;
};
