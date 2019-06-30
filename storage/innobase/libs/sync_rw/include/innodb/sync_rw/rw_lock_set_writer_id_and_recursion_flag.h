#pragma once

#include <innodb/univ/univ.h>

#include <innodb/thread/os_thread_get_curr_id.h>
#include <innodb/memory_check/memory_check.h>
#include <innodb/sync_rw/rw_lock_t.h>

#ifndef UNIV_LIBRARY

/** This function sets the lock->writer_thread and lock->recursive fields. For
platforms where we are using atomic builtins instead of lock->mutex it sets
the lock->writer_thread field using atomics to ensure memory ordering. Note
that it is assumed that the caller of this function effectively owns the lock
i.e.: nobody else is allowed to modify lock->writer_thread at this point in
time. The protocol is that lock->writer_thread MUST be updated BEFORE the
lock->recursive flag is set.
@param[in,out]	lock		lock to work on
@param[in]	recursive	true if recursion allowed */
UNIV_INLINE
void rw_lock_set_writer_id_and_recursion_flag(rw_lock_t *lock, bool recursive);

#endif /* UNIV_LIBRARY */


#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP

/** This function sets the lock->writer_thread and lock->recursive fields.
 For platforms where we are using atomic builtins instead of lock->mutex
 it sets the lock->writer_thread field using atomics to ensure memory
 ordering. Note that it is assumed that the caller of this function
 effectively owns the lock i.e.: nobody else is allowed to modify
 lock->writer_thread at this point in time.
 The protocol is that lock->writer_thread MUST be updated BEFORE the
 lock->recursive flag is set. */
UNIV_INLINE
void rw_lock_set_writer_id_and_recursion_flag(
    rw_lock_t *lock, /*!< in/out: lock to work on */
    bool recursive)  /*!< in: true if recursion
                     allowed */
{
  os_thread_id_t curr_thread = os_thread_get_curr_id();

#ifdef INNODB_RW_LOCKS_USE_ATOMICS
  os_thread_id_t local_thread;
  ibool success;

  /* Prevent Valgrind warnings about writer_thread being
  uninitialized.  It does not matter if writer_thread is
  uninitialized, because we are comparing writer_thread against
  itself, and the operation should always succeed. */
  UNIV_MEM_VALID(&lock->writer_thread, sizeof lock->writer_thread);

  local_thread = lock->writer_thread;
  success = os_compare_and_swap_thread_id(&lock->writer_thread, local_thread,
                                          curr_thread);
  ut_a(success);
  lock->recursive = recursive;

#else /* INNODB_RW_LOCKS_USE_ATOMICS */

  mutex_enter(&lock->mutex);
  lock->writer_thread = curr_thread;
  lock->recursive = recursive;
  mutex_exit(&lock->mutex);

#endif /* INNODB_RW_LOCKS_USE_ATOMICS */
}

#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */
