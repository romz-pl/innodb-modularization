#include <innodb/sync_rw/MutexMonitor.h>


#include <innodb/sync_mutex/mutex_enter.h>
#include <innodb/sync_mutex/mutex_exit.h>
#include <innodb/sync_rw/rw_lock_list_mutex.h>
#include <innodb/sync_rw/rw_lock_list.h>

/** Enable the mutex monitoring */
void MutexMonitor::enable() {
  /** Note: We don't add any latch meta-data after startup. Therefore
  there is no need to use a mutex here. */

  LatchMetaData::iterator end = latch_meta.end();

  for (LatchMetaData::iterator it = latch_meta.begin(); it != end; ++it) {
    if (*it != NULL) {
      (*it)->get_counter()->enable();
    }
  }
}

/** Disable the mutex monitoring */
void MutexMonitor::disable() {
  /** Note: We don't add any latch meta-data after startup. Therefore
  there is no need to use a mutex here. */

  LatchMetaData::iterator end = latch_meta.end();

  for (LatchMetaData::iterator it = latch_meta.begin(); it != end; ++it) {
    if (*it != NULL) {
      (*it)->get_counter()->disable();
    }
  }
}

/** Reset the mutex monitoring counters */
void MutexMonitor::reset() {
  /** Note: We don't add any latch meta-data after startup. Therefore
  there is no need to use a mutex here. */

  LatchMetaData::iterator end = latch_meta.end();

  for (LatchMetaData::iterator it = latch_meta.begin(); it != end; ++it) {
    if (*it != NULL) {
      (*it)->get_counter()->reset();
    }
  }

  mutex_enter(&rw_lock_list_mutex);

  for (rw_lock_t *rw_lock = UT_LIST_GET_FIRST(rw_lock_list); rw_lock != NULL;
       rw_lock = UT_LIST_GET_NEXT(list, rw_lock)) {
    rw_lock->count_os_wait = 0;
  }

  mutex_exit(&rw_lock_list_mutex);
}
