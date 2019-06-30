#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_event/os_event_t.h>
#include <innodb/thread/os_thread_id_t.h>
#include <innodb/lst/lst.h>
#include <innodb/sync_mutex/ib_mutex_t.h>


/* NOTE! The structure appears here only for the compiler to know its size.
Do not use its fields directly! */

/** The structure used in the spin lock implementation of a read-write
lock. Several threads may have a shared lock simultaneously in this
lock, but only one writer may have an exclusive lock, in which case no
shared locks are allowed. To prevent starving of a writer blocked by
readers, a writer may queue for x-lock by decrementing lock_word: no
new readers will be let in while the thread waits for readers to
exit. */

struct rw_lock_t
#ifdef UNIV_DEBUG
    : public latch_t
#endif /* UNIV_DEBUG */
{
  /** Holds the state of the lock. */
  volatile lint lock_word;

  /** 1: there are waiters */
  volatile ulint waiters;

  /** Default value FALSE which means the lock is non-recursive.
  The value is typically set to TRUE making normal rw_locks recursive.
  In case of asynchronous IO, when a non-zero value of 'pass' is
  passed then we keep the lock non-recursive.

  This flag also tells us about the state of writer_thread field.
  If this flag is set then writer_thread MUST contain the thread
  id of the current x-holder or wait-x thread.  This flag must be
  reset in x_unlock functions before incrementing the lock_word */
  volatile bool recursive;

  /** number of granted SX locks. */
  volatile ulint sx_recursive;

  /** This is TRUE if the writer field is RW_LOCK_X_WAIT; this field
  is located far from the memory update hotspot fields which are at
  the start of this struct, thus we can peek this field without
  causing much memory bus traffic */
  bool writer_is_wait_ex;

  /** Thread id of writer thread. Is only guaranteed to have sane
  and non-stale value iff recursive flag is set. */
  volatile os_thread_id_t writer_thread;

  /** Used by sync0arr.cc for thread queueing */
  os_event_t event;

  /** Event for next-writer to wait on. A thread must decrement
  lock_word before waiting. */
  os_event_t wait_ex_event;

  /** File name where lock created */
  const char *cfile_name;

  /** last s-lock file/line is not guaranteed to be correct */
  const char *last_s_file_name;

  /** File name where last x-locked */
  const char *last_x_file_name;

  /** Line where created */
  unsigned cline : 13;

  /** If 1 then the rw-lock is a block lock */
  unsigned is_block_lock : 1;

  /** Line number where last time s-locked */
  unsigned last_s_line : 14;

  /** Line number where last time x-locked */
  unsigned last_x_line : 14;

  /** Count of os_waits. May not be accurate */
  uint32_t count_os_wait;

  /** All allocated rw locks are put into a list */
  UT_LIST_NODE_T(rw_lock_t) list;

#ifdef UNIV_PFS_RWLOCK
  /** The instrumentation hook */
  struct PSI_rwlock *pfs_psi;
#endif /* UNIV_PFS_RWLOCK */

#ifndef INNODB_RW_LOCKS_USE_ATOMICS
  /** The mutex protecting rw_lock_t */
  mutable ib_mutex_t mutex;
#endif /* INNODB_RW_LOCKS_USE_ATOMICS */

#ifdef UNIV_DEBUG
/** Value of rw_lock_t::magic_n */
#define RW_LOCK_MAGIC_N 22643

  /** Constructor */
  rw_lock_t() { magic_n = RW_LOCK_MAGIC_N; }

  rw_lock_t &operator=(const rw_lock_t &) = default;

  /** Destructor */
  virtual ~rw_lock_t() {
    ut_ad(magic_n == RW_LOCK_MAGIC_N);
    magic_n = 0;
  }

  virtual std::string to_string() const;
  virtual std::string locked_from() const;

  /** For checking memory corruption. */
  ulint magic_n;

  /** In the debug version: pointer to the debug info list of the lock */
  UT_LIST_BASE_NODE_T(rw_lock_debug_t) debug_list;

  /** Level in the global latching order. */
  latch_level_t level;

#endif /* UNIV_DEBUG */
};


