#pragma once

#include <innodb/univ/univ.h>

#include <innodb/lock_sys/LockMutex.h>
#include <innodb/sync_event/os_event_t.h>
#include <innodb/cpu/flags.h>

struct hash_table_t;
struct srv_slot_t;

/** The lock system struct */
struct lock_sys_t {
  char pad1[INNOBASE_CACHE_LINE_SIZE];
  /*!< padding to prevent other
  memory update hotspots from
  residing on the same memory
  cache line */
  LockMutex mutex;              /*!< Mutex protecting the
                                locks */
  hash_table_t *rec_hash;       /*!< hash table of the record
                                locks */
  hash_table_t *prdt_hash;      /*!< hash table of the predicate
                                lock */
  hash_table_t *prdt_page_hash; /*!< hash table of the page
                                lock */

  char pad2[INNOBASE_CACHE_LINE_SIZE]; /*!< Padding */
  LockMutex wait_mutex;                /*!< Mutex protecting the
                                       next two fields */
  srv_slot_t *waiting_threads;         /*!< Array  of user threads
                                       suspended while waiting for
                                       locks within InnoDB, protected
                                       by the lock_sys->wait_mutex */
  srv_slot_t *last_slot;               /*!< highest slot ever used
                                       in the waiting_threads array,
                                       protected by
                                       lock_sys->wait_mutex */
  int n_waiting;                       /*!< Number of slots in use.
                                       Protected by lock_sys->mutex */
  ibool rollback_complete;
  /*!< TRUE if rollback of all
  recovered transactions is
  complete. Protected by
  lock_sys->mutex */

  ulint n_lock_max_wait_time; /*!< Max wait time */

  os_event_t timeout_event; /*!< Set to the event that is
                            created in the lock wait monitor
                            thread. A value of 0 means the
                            thread is not active */

  /** Marker value before trx_t::age. */
  uint64_t mark_age_updated;

#ifdef UNIV_DEBUG
  /** Lock timestamp counter */
  uint64_t m_seq;
#endif /* UNIV_DEBUG */
};
