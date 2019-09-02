#include <innodb/lock_sys/lock_sys_close.h>

#include <innodb/lock_sys/lock_latest_err_file.h>
#include <innodb/hash/hash_table_free.h>
#include <innodb/sync_event/os_event_destroy.h>
#include <innodb/sync_mutex/mutex_destroy.h>
#include <innodb/lock_sys/lock_sys.h>
#include <innodb/srv_thread/srv_slot_t.h>
#include <innodb/lock_sys/lock_cached_lock_mode_names.h>
#include <innodb/allocator/ut_free.h>

extern ulint srv_max_n_threads;

/** Closes the lock system at database shutdown. */
void lock_sys_close(void) {
  if (lock_latest_err_file != NULL) {
    fclose(lock_latest_err_file);
    lock_latest_err_file = NULL;
  }

  hash_table_free(lock_sys->rec_hash);
  hash_table_free(lock_sys->prdt_hash);
  hash_table_free(lock_sys->prdt_page_hash);

  os_event_destroy(lock_sys->timeout_event);

  mutex_destroy(&lock_sys->mutex);
  mutex_destroy(&lock_sys->wait_mutex);

  srv_slot_t *slot = lock_sys->waiting_threads;

  for (ulint i = 0; i < srv_max_n_threads; i++, ++slot) {
    if (slot->event != NULL) {
      os_event_destroy(slot->event);
    }
  }
  for (auto &cached_lock_mode_name : lock_cached_lock_mode_names) {
    ut_free(const_cast<char *>(cached_lock_mode_name.second));
  }
  lock_cached_lock_mode_names.clear();
  ut_free(lock_sys);

  lock_sys = NULL;
}
