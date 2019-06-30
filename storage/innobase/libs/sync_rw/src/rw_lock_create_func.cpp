#include <innodb/sync_rw/rw_lock_create_func.h>

#include <innodb/sync_rw/rw_lock_list_mutex.h>
#include <innodb/sync_rw/rw_lock_list.h>
#include <innodb/sync_rw/X_LOCK_DECR.h>
#include <innodb/memory_check/memory_check.h>
#include <innodb/sync_mutex/mutex_enter.h>
#include <innodb/sync_mutex/mutex_exit.h>

/** Creates, or rather, initializes an rw-lock object in a specified memory
 location (which must be appropriately aligned). The rw-lock is initialized
 to the non-locked state. Explicit freeing of the rw-lock with rw_lock_free
 is necessary only if the memory block containing it is freed. */
void rw_lock_create_func(
    rw_lock_t *lock, /*!< in: pointer to memory */
#ifdef UNIV_DEBUG
    latch_level_t level,     /*!< in: level */
    const char *cmutex_name, /*!< in: rw-lock name */
#endif                       /* UNIV_DEBUG */
    const char *cfile_name,  /*!< in: file name where created */
    ulint cline)             /*!< in: file line where created */
{
#if defined(UNIV_DEBUG)
#if !defined(UNIV_PFS_RWLOCK)
  /* It should have been created in pfs_rw_lock_create_func() */
  new (lock) rw_lock_t();
#endif /* UNIV_DEBUG */
  ut_ad(lock->magic_n == RW_LOCK_MAGIC_N);
#endif /* UNIV_DEBUG */

  /* If this is the very first time a synchronization object is
  created, then the following call initializes the sync system. */

#ifndef INNODB_RW_LOCKS_USE_ATOMICS
  mutex_create(LATCH_ID_RW_LOCK_MUTEX, rw_lock_get_mutex(lock));
#else /* INNODB_RW_LOCKS_USE_ATOMICS */
#ifdef UNIV_DEBUG
  UT_NOT_USED(cmutex_name);
#endif
#endif /* INNODB_RW_LOCKS_USE_ATOMICS */

  lock->lock_word = X_LOCK_DECR;
  lock->waiters = 0;

  /* We set this value to signify that lock->writer_thread
  contains garbage at initialization and cannot be used for
  recursive x-locking. */
  lock->recursive = FALSE;
  lock->sx_recursive = 0;
  /* Silence Valgrind when UNIV_DEBUG_VALGRIND is not enabled. */
  memset((void *)&lock->writer_thread, 0, sizeof lock->writer_thread);
  UNIV_MEM_INVALID(&lock->writer_thread, sizeof lock->writer_thread);

#ifdef UNIV_DEBUG
  lock->m_rw_lock = true;

  UT_LIST_INIT(lock->debug_list, &rw_lock_debug_t::list);

  lock->m_id = sync_latch_get_id(sync_latch_get_name(level));
  ut_a(lock->m_id != LATCH_ID_NONE);

  lock->level = level;
#endif /* UNIV_DEBUG */

  lock->cfile_name = cfile_name;

  /* This should hold in practice. If it doesn't then we need to
  split the source file anyway. Or create the locks on lines
  less than 8192. cline is unsigned:13. */
  ut_ad(cline <= 8192);
  lock->cline = (unsigned int)cline;

  lock->count_os_wait = 0;
  lock->last_s_file_name = "not yet reserved";
  lock->last_x_file_name = "not yet reserved";
  lock->last_s_line = 0;
  lock->last_x_line = 0;
  lock->event = os_event_create(0);
  lock->wait_ex_event = os_event_create(0);

  lock->is_block_lock = 0;

  mutex_enter(&rw_lock_list_mutex);

  ut_ad(UT_LIST_GET_FIRST(rw_lock_list) == NULL ||
        UT_LIST_GET_FIRST(rw_lock_list)->magic_n == RW_LOCK_MAGIC_N);

  UT_LIST_ADD_FIRST(rw_lock_list, lock);

  mutex_exit(&rw_lock_list_mutex);
}
