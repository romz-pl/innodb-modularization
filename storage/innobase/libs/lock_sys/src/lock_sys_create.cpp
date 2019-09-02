#include <innodb/lock_sys/lock_sys_create.h>

#include <innodb/srv_thread/srv_slot_t.h>
#include <innodb/lock_sys/lock_sys.h>
#include <innodb/allocator/ut_zalloc_nokey.h>
#include <innodb/sync_mutex/mutex_create.h>
#include <innodb/hash/hash_create.h>
#include <innodb/io/os_file_create_tmpfile.h>
#include <innodb/io/srv_read_only_mode.h>
#include <innodb/lock_sys/lock_latest_err_file.h>

extern ulint srv_max_n_threads;


/** Creates the lock system at database start. */
void lock_sys_create(
    ulint n_cells) /*!< in: number of slots in lock hash table */
{
  ulint lock_sys_sz;

  lock_sys_sz = sizeof(*lock_sys) + srv_max_n_threads * sizeof(srv_slot_t);

  lock_sys = static_cast<lock_sys_t *>(ut_zalloc_nokey(lock_sys_sz));

  void *ptr = &lock_sys[1];

  lock_sys->waiting_threads = static_cast<srv_slot_t *>(ptr);

  lock_sys->last_slot = lock_sys->waiting_threads;

  mutex_create(LATCH_ID_LOCK_SYS, &lock_sys->mutex);

  mutex_create(LATCH_ID_LOCK_SYS_WAIT, &lock_sys->wait_mutex);

  lock_sys->timeout_event = os_event_create(0);

  lock_sys->rec_hash = hash_create(n_cells);
  lock_sys->prdt_hash = hash_create(n_cells);
  lock_sys->prdt_page_hash = hash_create(n_cells);

  if (!srv_read_only_mode) {
    lock_latest_err_file = os_file_create_tmpfile(NULL);
    ut_a(lock_latest_err_file);
  }
}
