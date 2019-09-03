#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_types/trx_que_t.h>
#include <innodb/trx_trx/lock_pool_t.h>
#include <innodb/memory/mem_heap_t.h>
#include <innodb/lock_types/trx_lock_list_t.h>


struct lock_t;
struct que_thr_t;

/** The locks and state of an active transaction. Protected by
lock_sys->mutex, trx->mutex or both. */
struct trx_lock_t {
  ulint n_active_thrs; /*!< number of active query threads */

  trx_que_t que_state; /*!< valid when trx->state
                       == TRX_STATE_ACTIVE: TRX_QUE_RUNNING,
                       TRX_QUE_LOCK_WAIT, ... */

  lock_t *wait_lock;         /*!< if trx execution state is
                             TRX_QUE_LOCK_WAIT, this points to
                             the lock request, otherwise this is
                             NULL; set to non-NULL when holding
                             both trx->mutex and lock_sys->mutex;
                             set to NULL when holding
                             lock_sys->mutex; readers should
                             hold lock_sys->mutex, except when
                             they are holding trx->mutex and
                             wait_lock==NULL */
  ib_uint64_t deadlock_mark; /*!< A mark field that is initialized
                             to and checked against lock_mark_counter
                             by lock_deadlock_recursive(). */
  bool was_chosen_as_deadlock_victim;
  /*!< when the transaction decides to
  wait for a lock, it sets this to false;
  if another transaction chooses this
  transaction as a victim in deadlock
  resolution, it sets this to true.
  Protected by trx->mutex. */
  time_t wait_started; /*!< lock wait started at this time,
                       protected only by lock_sys->mutex */

  que_thr_t *wait_thr; /*!< query thread belonging to this
                       trx that is in QUE_THR_LOCK_WAIT
                       state. For threads suspended in a
                       lock wait, this is protected by
                       lock_sys->mutex. Otherwise, this may
                       only be modified by the thread that is
                       serving the running transaction. */

  lock_pool_t rec_pool; /*!< Pre-allocated record locks */

  lock_pool_t table_pool; /*!< Pre-allocated table locks */

  ulint rec_cached; /*!< Next free rec lock in pool */

  ulint table_cached; /*!< Next free table lock in pool */

  mem_heap_t *lock_heap; /*!< memory heap for trx_locks;
                         protected by lock_sys->mutex */

  trx_lock_list_t trx_locks; /*!< locks requested by the transaction;
                             insertions are protected by trx->mutex
                             and lock_sys->mutex; removals are
                             protected by lock_sys->mutex */

  lock_pool_t table_locks; /*!< All table locks requested by this
                           transaction, including AUTOINC locks */

  ulint n_rec_locks; /*!< number of rec locks in this trx */
#ifdef UNIV_DEBUG
  /** When a transaction is forced to rollback due to a deadlock
  check or by another high priority transaction this is true. Used
  by debug checks in lock0lock.cc */
  bool in_rollback;
#endif /* UNIV_DEBUG */

  /** The transaction called ha_innobase::start_stmt() to
  lock a table. Most likely a temporary table. */
  bool start_stmt;
};
