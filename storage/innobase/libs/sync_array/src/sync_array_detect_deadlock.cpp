#include <innodb/sync_array/sync_array_detect_deadlock.h>

#ifdef UNIV_DEBUG

/** This function is called only in the debug version. Detects a deadlock
 of one or more threads because of waits of semaphores.
 @return true if deadlock detected */
bool sync_array_detect_deadlock(
    sync_array_t *arr,  /*!< in: wait array; NOTE! the caller must
                        own the mutex to array */
    sync_cell_t *start, /*!< in: cell where recursive search started */
    sync_cell_t *cell,  /*!< in: cell to search */
    ulint depth)        /*!< in: recursion depth */
{
  rw_lock_t *lock;
  os_thread_id_t thread;
  ibool ret;
  rw_lock_debug_t *debug;

  ut_a(arr);
  ut_a(start);
  ut_a(cell);
  ut_ad(cell->latch.mutex != 0);
  ut_ad(os_thread_get_curr_id() == start->thread_id);
  ut_ad(depth < 100);

  depth++;

  if (!cell->waiting) {
    /* No deadlock here */
    return (false);
  }

  switch (cell->request_type) {
    case SYNC_MUTEX: {
      WaitMutex *mutex = cell->latch.mutex;
      const WaitMutex::MutexPolicy &policy = mutex->policy();

      if (mutex->state() != MUTEX_STATE_UNLOCKED) {
        thread = policy.get_thread_id();

        /* Note that mutex->thread_id above may be
        also OS_THREAD_ID_UNDEFINED, because the
        thread which held the mutex maybe has not
        yet updated the value, or it has already
        released the mutex: in this case no deadlock
        can occur, as the wait array cannot contain
        a thread with ID_UNDEFINED value. */
        ret = sync_array_deadlock_step(arr, start, thread, 0, depth);

        if (ret) {
          const char *name;

          name = policy.get_enter_filename();

          if (name == NULL) {
            /* The mutex might have been
            released. */
            name = "NULL";
          }

#ifdef UNIV_NO_ERR_MSGS
          ib::info()
#else
          ib::info(ER_IB_MSG_1158)
#endif /* UNIV_NO_ERR_MSGS */
              << "Mutex " << mutex
              << " owned by"
                 " thread "
              << thread << " file " << name << " line "
              << policy.get_enter_line();

          sync_array_cell_print(stderr, cell);

          return (true);
        }
      }

      /* No deadlock */
      return (false);
    }

    case SYNC_BUF_BLOCK: {
      BlockWaitMutex *mutex = cell->latch.bpmutex;

      const BlockWaitMutex::MutexPolicy &policy = mutex->policy();

      if (mutex->state() != MUTEX_STATE_UNLOCKED) {
        thread = policy.get_thread_id();

        /* Note that mutex->thread_id above may be
        also OS_THREAD_ID_UNDEFINED, because the
        thread which held the mutex maybe has not
        yet updated the value, or it has already
        released the mutex: in this case no deadlock
        can occur, as the wait array cannot contain
        a thread with ID_UNDEFINED value. */
        ret = sync_array_deadlock_step(arr, start, thread, 0, depth);

        if (ret) {
          const char *name;

          name = policy.get_enter_filename();

          if (name == NULL) {
            /* The mutex might have been
            released. */
            name = "NULL";
          }

#ifdef UNIV_NO_ERR_MSGS
          ib::info()
#else
          ib::info(ER_IB_MSG_1159)
#endif /* UNIV_NO_ERR_MSGS */
              << "Mutex " << mutex
              << " owned by"
                 " thread "
              << thread << " file " << name << " line "
              << policy.get_enter_line();

          sync_array_cell_print(stderr, cell);

          return (true);
        }
      }

      /* No deadlock */
      return (false);
    }
    case RW_LOCK_X:
    case RW_LOCK_X_WAIT:

      lock = cell->latch.lock;

      for (debug = UT_LIST_GET_FIRST(lock->debug_list); debug != NULL;
           debug = UT_LIST_GET_NEXT(list, debug)) {
        thread = debug->thread_id;

        switch (debug->lock_type) {
          case RW_LOCK_X:
          case RW_LOCK_SX:
          case RW_LOCK_X_WAIT:
            if (os_thread_eq(thread, cell->thread_id)) {
              break;
            }
            /* fall through */
          case RW_LOCK_S:

            /* The (wait) x-lock request can block
            infinitely only if someone (can be also cell
            thread) is holding s-lock, or someone
            (cannot be cell thread) (wait) x-lock or
            sx-lock, and he is blocked by start thread */

            ret = sync_array_deadlock_step(arr, start, thread, debug->pass,
                                           depth);

            if (ret) {
              sync_array_report_error(lock, debug, cell);
              rw_lock_debug_print(stderr, debug);
              return (TRUE);
            }
        }
      }

      return (false);

    case RW_LOCK_SX:

      lock = cell->latch.lock;

      for (debug = UT_LIST_GET_FIRST(lock->debug_list); debug != 0;
           debug = UT_LIST_GET_NEXT(list, debug)) {
        thread = debug->thread_id;

        switch (debug->lock_type) {
          case RW_LOCK_X:
          case RW_LOCK_SX:
          case RW_LOCK_X_WAIT:

            if (os_thread_eq(thread, cell->thread_id)) {
              break;
            }

            /* The sx-lock request can block infinitely
            only if someone (can be also cell thread) is
            holding (wait) x-lock or sx-lock, and he is
            blocked by start thread */

            ret = sync_array_deadlock_step(arr, start, thread, debug->pass,
                                           depth);

            if (ret) {
              sync_array_report_error(lock, debug, cell);
              return (TRUE);
            }
        }
      }

      return (false);

    case RW_LOCK_S:

      lock = cell->latch.lock;

      for (debug = UT_LIST_GET_FIRST(lock->debug_list); debug != 0;
           debug = UT_LIST_GET_NEXT(list, debug)) {
        thread = debug->thread_id;

        if (debug->lock_type == RW_LOCK_X ||
            debug->lock_type == RW_LOCK_X_WAIT) {
          /* The s-lock request can block infinitely
          only if someone (can also be cell thread) is
          holding (wait) x-lock, and he is blocked by
          start thread */

          ret =
              sync_array_deadlock_step(arr, start, thread, debug->pass, depth);

          if (ret) {
            sync_array_report_error(lock, debug, cell);
            return (TRUE);
          }
        }
      }

      return (false);

    default:
      ut_error;
  }

  return (true);
}


#endif
