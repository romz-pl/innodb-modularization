/*****************************************************************************

Copyright (c) 1996, 2018, Oracle and/or its affiliates. All Rights Reserved.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2.0, as published by the
Free Software Foundation.

This program is also distributed with certain software (including but not
limited to OpenSSL) that is licensed under separate terms, as designated in a
particular file or component or in included license documentation. The authors
of MySQL hereby grant you an additional permission to link the program and
your derivative works with the separately licensed software that they have
included with MySQL.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License, version 2.0,
for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

*****************************************************************************/

/** @file include/trx0trx.h
 The transaction

 Created 3/26/1996 Heikki Tuuri
 *******************************************************/

#ifndef trx0trx_h
#define trx0trx_h

#include <innodb/univ/univ.h>

#include <innodb/trx_types/trx_savept_t.h>
#include <innodb/trx_types/UndoMutex.h>
#include <innodb/trx_types/TrxMutex.h>
#include <innodb/trx_types/trx_que_t.h>
#include <innodb/trx_types/trx_state_t.h>
#include <innodb/trx_types/trx_dict_op_t.h>
#include <innodb/lock_types/trx_lock_list_t.h>
#include <innodb/sync_mutex/mutex_enter.h>
#include <innodb/sync_mutex/mutex_exit.h>
#include <innodb/io/srv_read_only_mode.h>
#include <innodb/trx_trx/trx_state_eq.h>
#include <innodb/trx_trx/trx_get_error_index.h>
#include <innodb/trx_trx/trx_get_dict_operation.h>
#include <innodb/trx_trx/trx_set_dict_operation.h>
#include <innodb/data_types/flags.h>
#include <innodb/page/flag.h>
#include <innodb/read/MVCC.h>
#include <innodb/trx_trx/trx_is_redo_rseg_updated.h>
#include <innodb/trx_trx/trx_is_temp_rseg_updated.h>
#include <innodb/trx_trx/trx_is_rseg_updated.h>
#include <innodb/trx_trx/trx_is_rseg_assigned.h>
#include <innodb/trx_trx/trx_get_que_state_str.h>
#include <innodb/trx_trx/trx_get_id_for_print.h>
#include <innodb/trx_trx/trx_reference.h>
#include <innodb/trx_trx/trx_release_reference.h>
#include <innodb/trx_trx/trx_is_referenced.h>
#include <innodb/trx_trx/trx_arbitrate.h>
#include <innodb/trx_trx/trx_is_high_priority.h>
#include <innodb/trx_trx/trx_is_rseg_updated.h>
#include <innodb/trx_trx/trx_is_autocommit_non_locking.h>
#include <innodb/trx_trx/trx_is_ac_nl_ro.h>
#include <innodb/trx_trx/assert_trx_in_rw_list.h>
#include <innodb/trx_trx/check_trx_state.h>
#include <innodb/trx_trx/lock_pool_t.h>
#include <innodb/trx_trx/trx_mod_tables_t.h>
#include <innodb/trx_trx/trx_undo_ptr_t.h>
#include <innodb/trx_trx/trx_rsegs_t.h>
#include <innodb/trx_trx/TrxVersion.h>
#include <innodb/trx_trx/hit_list_t.h>
#include <innodb/trx_trx/trx_t.h>
#include <innodb/trx_trx/trx_mutex_own.h>
#include <innodb/trx_trx/trx_mutex_enter.h>
#include <innodb/trx_trx/trx_mutex_exit.h>

#include <list>
#include <set>

#include "ha_prototypes.h"


struct lock_t;
struct lock_sys_t;
struct lock_table_t;

#include "log0log.h"

#include "que0types.h"
#include "trx0xa.h"
#include "usr0types.h"
#include <innodb/vector/vector.h>
#ifndef UNIV_HOTBACKUP
#include "fts0fts.h"
#endif /* !UNIV_HOTBACKUP */
#include "srv0srv.h"

struct trx_undo_t;
struct trx_rseg_t;
struct trx_named_savept_t;
struct commit_node_t;

// Forward declaration
struct mtr_t;

// Forward declaration
class ReadView;

// Forward declaration
class FlushObserver;

/** Dummy session used currently in MySQL interface */
extern sess_t *trx_dummy_sess;

#ifndef UNIV_HOTBACKUP
/** Set flush observer for the transaction
@param[in,out]	trx		transaction struct
@param[in]	observer	flush observer */
void trx_set_flush_observer(trx_t *trx, FlushObserver *observer);

/** Set detailed error message for the transaction. */
void trx_set_detailed_error(trx_t *trx,       /*!< in: transaction struct */
                            const char *msg); /*!< in: detailed error message */
/** Set detailed error message for the transaction from a file. Note that the
 file is rewinded before reading from it. */
void trx_set_detailed_error_from_file(
    trx_t *trx,  /*!< in: transaction struct */
    FILE *file); /*!< in: file to read message from */

/** Creates a transaction object for MySQL.
 @return own: transaction object */
trx_t *trx_allocate_for_mysql(void);
/** Creates a transaction object for background operations by the master thread.
 @return own: transaction object */
trx_t *trx_allocate_for_background(void);

/** Resurrect table locks for resurrected transactions. */
void trx_resurrect_locks();

/** Free and initialize a transaction object instantiated during recovery.
@param[in,out]	trx	transaction object to free and initialize */
void trx_free_resurrected(trx_t *trx);

/** Free a transaction that was allocated by background or user threads.
@param[in,out]	trx	transaction object to free */
void trx_free_for_background(trx_t *trx);

/** At shutdown, frees a transaction object that is in the PREPARED state. */
void trx_free_prepared(trx_t *trx); /*!< in, own: trx object */

/** Free a transaction object for MySQL.
@param[in,out]	trx	transaction */
void trx_free_for_mysql(trx_t *trx);

/** Disconnect a transaction from MySQL.
@param[in,out]	trx	transaction */
void trx_disconnect_plain(trx_t *trx);

/** Disconnect a prepared transaction from MySQL.
@param[in,out]	trx	transaction */
void trx_disconnect_prepared(trx_t *trx);

/** Creates trx objects for transactions and initializes the trx list of
 trx_sys at database start. Rollback segment and undo log lists must
 already exist when this function is called, because the lists of
 transactions to be rolled back or cleaned up are built based on the
 undo log lists. */
void trx_lists_init_at_db_start(void);

/** Starts the transaction if it is not yet started. */
void trx_start_if_not_started_xa_low(
    trx_t *trx,       /*!< in/out: transaction */
    bool read_write); /*!< in: true if read write transaction */
/** Starts the transaction if it is not yet started. */
void trx_start_if_not_started_low(
    trx_t *trx,       /*!< in/out: transaction */
    bool read_write); /*!< in: true if read write transaction */

/** Starts a transaction for internal processing. */
void trx_start_internal_low(trx_t *trx); /*!< in/out: transaction */

/** Starts a read-only transaction for internal processing.
@param[in,out] trx	transaction to be started */
void trx_start_internal_read_only_low(trx_t *trx);

#ifdef UNIV_DEBUG
#define trx_start_if_not_started_xa(t, rw)    \
  do {                                        \
    (t)->start_line = __LINE__;               \
    (t)->start_file = __FILE__;               \
    trx_start_if_not_started_xa_low((t), rw); \
  } while (false)

#define trx_start_if_not_started(t, rw)    \
  do {                                     \
    (t)->start_line = __LINE__;            \
    (t)->start_file = __FILE__;            \
    trx_start_if_not_started_low((t), rw); \
  } while (false)

#define trx_start_internal(t)    \
  do {                           \
    (t)->start_line = __LINE__;  \
    (t)->start_file = __FILE__;  \
    trx_start_internal_low((t)); \
  } while (false)

#define trx_start_internal_read_only(t)  \
  do {                                   \
    (t)->start_line = __LINE__;          \
    (t)->start_file = __FILE__;          \
    trx_start_internal_read_only_low(t); \
  } while (false)
#else
#define trx_start_if_not_started(t, rw) trx_start_if_not_started_low((t), rw)

#define trx_start_internal(t) trx_start_internal_low((t))

#define trx_start_internal_read_only(t) trx_start_internal_read_only_low(t)

#define trx_start_if_not_started_xa(t, rw) \
  trx_start_if_not_started_xa_low((t), (rw))
#endif /* UNIV_DEBUG */

/** Commits a transaction. */
void trx_commit(trx_t *trx); /*!< in/out: transaction */

/** Commits a transaction and a mini-transaction. */
void trx_commit_low(
    trx_t *trx,  /*!< in/out: transaction */
    mtr_t *mtr); /*!< in/out: mini-transaction (will be committed),
                 or NULL if trx made no modifications */
/** Cleans up a transaction at database startup. The cleanup is needed if
 the transaction already got to the middle of a commit when the database
 crashed, and we cannot roll it back. */
void trx_cleanup_at_db_startup(trx_t *trx); /*!< in: transaction */
/** Does the transaction commit for MySQL.
 @return DB_SUCCESS or error number */
dberr_t trx_commit_for_mysql(trx_t *trx); /*!< in/out: transaction */

/**
Does the transaction prepare for MySQL.
@param[in, out] trx		Transaction instance to prepare */

dberr_t trx_prepare_for_mysql(trx_t *trx);

/** This function is used to find number of prepared transactions and
 their transaction objects for a recovery.
 @return number of prepared transactions */
int trx_recover_for_mysql(
    XA_recover_txn *txn_list, /*!< in/out: prepared transactions */
    ulint len,                /*!< in: number of slots in xid_list */
    MEM_ROOT *mem_root);      /*!< in: memory for table names */
/** This function is used to find one X/Open XA distributed transaction
 which is in the prepared state
 @return trx or NULL; on match, the trx->xid will be invalidated;
 note that the trx may have been committed, unless the caller is
 holding lock_sys->mutex */
trx_t *trx_get_trx_by_xid(
    const XID *xid); /*!< in: X/Open XA transaction identifier */
/** If required, flushes the log to disk if we called trx_commit_for_mysql()
 with trx->flush_log_later == TRUE. */
void trx_commit_complete_for_mysql(trx_t *trx); /*!< in/out: transaction */
/** Marks the latest SQL statement ended. */
void trx_mark_sql_stat_end(trx_t *trx); /*!< in: trx handle */
/** Assigns a read view for a consistent read query. All the consistent reads
 within the same transaction will get the same read view, which is created
 when this function is first called for a new started transaction. */
ReadView *trx_assign_read_view(trx_t *trx); /*!< in: active transaction */

/** @return the transaction's read view or NULL if one not assigned. */
UNIV_INLINE
ReadView *trx_get_read_view(trx_t *trx);

/** @return the transaction's read view or NULL if one not assigned. */
UNIV_INLINE
const ReadView *trx_get_read_view(const trx_t *trx);

/** Prepares a transaction for commit/rollback. */
void trx_commit_or_rollback_prepare(trx_t *trx); /*!< in/out: transaction */
/** Creates a commit command node struct.
 @return own: commit node struct */
commit_node_t *trx_commit_node_create(
    mem_heap_t *heap); /*!< in: mem heap where created */
/** Performs an execution step for a commit type node in a query graph.
 @return query thread to run next, or NULL */
que_thr_t *trx_commit_step(que_thr_t *thr); /*!< in: query thread */

/** Prints info about a transaction.
 Caller must hold trx_sys->mutex. */
void trx_print_low(FILE *f,
                   /*!< in: output stream */
                   const trx_t *trx,
                   /*!< in: transaction */
                   ulint max_query_len,
                   /*!< in: max query length to print,
                   or 0 to use the default max length */
                   ulint n_rec_locks,
                   /*!< in: lock_number_of_rows_locked(&trx->lock) */
                   ulint n_trx_locks,
                   /*!< in: length of trx->lock.trx_locks */
                   ulint heap_size);
/*!< in: mem_heap_get_size(trx->lock.lock_heap) */

/** Prints info about a transaction.
 The caller must hold lock_sys->mutex and trx_sys->mutex.
 When possible, use trx_print() instead. */
void trx_print_latched(
    FILE *f,              /*!< in: output stream */
    const trx_t *trx,     /*!< in: transaction */
    ulint max_query_len); /*!< in: max query length to print,
                          or 0 to use the default max length */

/** Prints info about a transaction.
 Acquires and releases lock_sys->mutex and trx_sys->mutex. */
void trx_print(FILE *f,              /*!< in: output stream */
               const trx_t *trx,     /*!< in: transaction */
               ulint max_query_len); /*!< in: max query length to print,
                                     or 0 to use the default max length */






#ifdef UNIV_DEBUG
/** Asserts that a transaction has been started.
 The caller must hold trx_sys->mutex.
 @return true if started */
ibool trx_assert_started(const trx_t *trx) /*!< in: transaction */
    MY_ATTRIBUTE((warn_unused_result));
#endif /* UNIV_DEBUG */

/** Determines if the currently running transaction has been interrupted.
 @return true if interrupted */
ibool trx_is_interrupted(const trx_t *trx); /*!< in: transaction */
/** Determines if the currently running transaction is in strict mode.
 @return true if strict */
ibool trx_is_strict(trx_t *trx); /*!< in: transaction */

/** Calculates the "weight" of a transaction. The weight of one transaction
 is estimated as the number of altered rows + the number of locked rows.
 @param t transaction
 @return transaction weight */
#define TRX_WEIGHT(t) ((t)->undo_no + UT_LIST_GET_LEN((t)->lock.trx_locks))

/** Compares the "weight" (or size) of two transactions. Transactions that
 have edited non-transactional tables are considered heavier than ones
 that have not.
 @return true if weight(a) >= weight(b) */
bool trx_weight_ge(const trx_t *a,  /*!< in: the transaction to be compared */
                   const trx_t *b); /*!< in: the transaction to be compared */
/* Maximum length of a string that can be returned by
trx_get_que_state_str(). */
#define TRX_QUE_STATE_STR_MAX_LEN 12 /* "ROLLING BACK" */



/** Assign a temp-tablespace bound rollback-segment to a transaction.
@param[in,out]	trx	transaction that involves write to temp-table. */
void trx_assign_rseg_temp(trx_t *trx);

/** Create the trx_t pool */
void trx_pool_init();

/** Destroy the trx_t pool */
void trx_pool_close();

/**
Set the transaction as a read-write transaction if it is not already
tagged as such.
@param[in,out] trx	Transaction that needs to be "upgraded" to RW from RO */
void trx_set_rw_mode(trx_t *trx);



/**
Kill all transactions that are blocking this transaction from acquiring locks.
@param[in,out] trx	High priority transaction */

void trx_kill_blocking(trx_t *trx);

/** Provides an id of the transaction which does not change over time.
Contrast this with trx->id and trx_get_id_for_print(trx) which change value once
a transaction can no longer be treated as read-only and becomes read-write.
@param[in]  trx   The transaction for which you want an immutable id
@return the transaction's immutable id */
UNIV_INLINE
uint64_t trx_immutable_id(const trx_t *trx) {
  return reinterpret_cast<uint64_t>(trx);
}


/**
Transactions that aren't started by the MySQL server don't set
the trx_t::mysql_thd field. For such transactions we set the lock
wait timeout to 0 instead of the user configured value that comes
from innodb_lock_wait_timeout via trx_t::mysql_thd.
@param	t transaction
@return lock wait timeout in seconds */
#define trx_lock_wait_timeout_get(t) thd_lock_wait_timeout((t)->mysql_thd)







/** Check if transaction is free so that it can be re-initialized.
@param t transaction handle */
#define assert_trx_is_free(t)                            \
  do {                                                   \
    ut_ad(trx_state_eq((t), TRX_STATE_NOT_STARTED) ||    \
          trx_state_eq((t), TRX_STATE_FORCED_ROLLBACK)); \
    ut_ad(!trx_is_rseg_updated(trx));                    \
    ut_ad(!MVCC::is_view_active((t)->read_view));        \
    ut_ad((t)->lock.wait_thr == NULL);                   \
    ut_ad(UT_LIST_GET_LEN((t)->lock.trx_locks) == 0);    \
    ut_ad((t)->dict_operation == TRX_DICT_OP_NONE);      \
  } while (0)

/** Check if transaction is in-active so that it can be freed and put back to
transaction pool.
@param t transaction handle */
#define assert_trx_is_inactive(t)              \
  do {                                         \
    assert_trx_is_free((t));                   \
    ut_ad((t)->dict_operation_lock_mode == 0); \
  } while (0)

#ifdef UNIV_DEBUG
/** Assert that an autocommit non-locking select cannot be in the
 rw_trx_list and that it is a read-only transaction.
 The tranasction must be in the mysql_trx_list. */
#define assert_trx_nonlocking_or_in_list(t)         \
  do {                                              \
    if (trx_is_autocommit_non_locking(t)) {         \
      trx_state_t t_state = (t)->state;             \
      ut_ad((t)->read_only);                        \
      ut_ad(!(t)->is_recovered);                    \
      ut_ad(!(t)->in_rw_trx_list);                  \
      ut_ad((t)->in_mysql_trx_list);                \
      ut_ad(t_state == TRX_STATE_NOT_STARTED ||     \
            t_state == TRX_STATE_FORCED_ROLLBACK || \
            t_state == TRX_STATE_ACTIVE);           \
    } else {                                        \
      check_trx_state(t);                           \
    }                                               \
  } while (0)
#else /* UNIV_DEBUG */
/** Assert that an autocommit non-locking slect cannot be in the
 rw_trx_list and that it is a read-only transaction.
 The tranasction must be in the mysql_trx_list. */
#define assert_trx_nonlocking_or_in_list(trx) ((void)0)
#endif /* UNIV_DEBUG */
#endif /* !UNIV_HOTBACKUP */



/** Latching protocol for trx_lock_t::que_state.  trx_lock_t::que_state
 captures the state of the query thread during the execution of a query.
 This is different from a transaction state. The query state of a transaction
 can be updated asynchronously by other threads.  The other threads can be
 system threads, like the timeout monitor thread or user threads executing
 other queries. Another thing to be mindful of is that there is a delay between
 when a query thread is put into LOCK_WAIT state and before it actually starts
 waiting.  Between these two events it is possible that the query thread is
 granted the lock it was waiting for, which implies that the state can be
 changed asynchronously.

 All these operations take place within the context of locking. Therefore state
 changes within the locking code must acquire both the lock mutex and the
 trx->mutex when changing trx->lock.que_state to TRX_QUE_LOCK_WAIT or
 trx->lock.wait_lock to non-NULL but when the lock wait ends it is sufficient
 to only acquire the trx->mutex.
 To query the state either of the mutexes is sufficient within the locking
 code and no mutex is required when the query thread is no longer waiting. */




/** The transaction handle

Normally, there is a 1:1 relationship between a transaction handle
(trx) and a session (client connection). One session is associated
with exactly one user transaction. There are some exceptions to this:

* For DDL operations, a subtransaction is allocated that modifies the
data dictionary tables. Lock waits and deadlocks are prevented by
acquiring the dict_operation_lock before starting the subtransaction
and releasing it after committing the subtransaction.

* The purge system uses a special transaction that is not associated
with any session.

* If the system crashed or it was quickly shut down while there were
transactions in the ACTIVE or PREPARED state, these transactions would
no longer be associated with a session when the server is restarted.

A session may be served by at most one thread at a time. The serving
thread of a session might change in some MySQL implementations.
Therefore we do not have os_thread_get_curr_id() assertions in the code.

Normally, only the thread that is currently associated with a running
transaction may access (read and modify) the trx object, and it may do
so without holding any mutex. The following are exceptions to this:

* trx_rollback_resurrected() may access resurrected (connectionless)
transactions while the system is already processing new user
transactions. The trx_sys->mutex prevents a race condition between it
and lock_trx_release_locks() [invoked by trx_commit()].

* Print of transactions may access transactions not associated with
the current thread. The caller must be holding trx_sys->mutex and
lock_sys->mutex.

* When a transaction handle is in the trx_sys->mysql_trx_list or
trx_sys->trx_list, some of its fields must not be modified without
holding trx_sys->mutex exclusively.

* The locking code (in particular, deadlock checking and implicit to
explicit conversion) will access transactions associated to other
connections. The locks of transactions are protected by lock_sys->mutex
and sometimes by trx->mutex.

* Killing of asynchronous transactions. */




enum trx_rseg_type_t {
  TRX_RSEG_TYPE_NONE = 0, /*!< void rollback segment type. */
  TRX_RSEG_TYPE_REDO,     /*!< redo rollback segment. */
  TRX_RSEG_TYPE_NOREDO    /*!< non-redo rollback segment. */
};




#ifndef UNIV_HOTBACKUP

/* Transaction isolation levels (trx->isolation_level) */
#define TRX_ISO_READ_UNCOMMITTED trx_t::READ_UNCOMMITTED
#define TRX_ISO_READ_COMMITTED trx_t::READ_COMMITTED
#define TRX_ISO_REPEATABLE_READ trx_t::REPEATABLE_READ
#define TRX_ISO_SERIALIZABLE trx_t::SERIALIZABLE

/**
Check if transaction is started.
@param[in] trx		Transaction whose state we need to check
@return true if transaction is in state started */
inline bool trx_is_started(const trx_t *trx) {
  return (trx->state != TRX_STATE_NOT_STARTED &&
          trx->state != TRX_STATE_FORCED_ROLLBACK);
}

/* Treatment of duplicate values (trx->duplicates; for example, in inserts).
Multiple flags can be combined with bitwise OR. */
#define TRX_DUP_IGNORE 1  /* duplicate rows are to be updated */
#define TRX_DUP_REPLACE 2 /* duplicate rows are to be replaced */

/** Commit node states */
enum commit_node_state {
  COMMIT_NODE_SEND = 1, /*!< about to send a commit signal to
                        the transaction */
  COMMIT_NODE_WAIT      /*!< commit signal sent to the transaction,
                        waiting for completion */
};

/** Commit command node in a query graph */
struct commit_node_t {
  que_common_t common;          /*!< node type: QUE_NODE_COMMIT */
  enum commit_node_state state; /*!< node execution state */
};







/** Track if a transaction is executing inside InnoDB code. It acts
like a gate between the Server and InnoDB.  */
class TrxInInnoDB {
 public:
  /**
  @param[in,out] trx	Transaction entering InnoDB via the handler
  @param[in] disable	true if called from COMMIT/ROLLBACK method */
  TrxInInnoDB(trx_t *trx, bool disable = false) : m_trx(trx) {
    enter(trx, disable);
  }

  /**
  Destructor */
  ~TrxInInnoDB() { exit(m_trx); }

  /**
  @return true if the transaction has been marked for asynchronous
          rollback */
  bool is_aborted() const { return (is_aborted(m_trx)); }

  /**
  @return true if the transaction can't be rolled back asynchronously */
  bool is_rollback_disabled() const {
    return ((m_trx->in_innodb & TRX_FORCE_ROLLBACK_DISABLE) > 0);
  }

  /**
  @return true if the transaction has been marked for asynchronous
          rollback */
  static bool is_aborted(const trx_t *trx) {
    if (trx->state == TRX_STATE_NOT_STARTED) {
      return (false);
    }

    ut_ad(srv_read_only_mode || trx->in_depth > 0);
    ut_ad(srv_read_only_mode || trx->in_innodb > 0);

    return (trx->abort || trx->state == TRX_STATE_FORCED_ROLLBACK);
  }

  /**
  Start statement requested for transaction.
  @param[in, out] trx	Transaction at the start of a SQL statement */
  static void begin_stmt(trx_t *trx) { enter(trx, false); }

  /**
  Note an end statement for transaction
  @param[in, out] trx	Transaction at end of a SQL statement */
  static void end_stmt(trx_t *trx) { exit(trx); }

  /**
  @return true if the rollback is being initiated by the thread that
          marked the transaction for asynchronous rollback */
  static bool is_async_rollback(const trx_t *trx) {
    return (trx->killed_by == os_thread_get_curr_id());
  }

 private:
  /** Note that we have crossed into InnoDB code.
  @param[in]	trx	transaction
  @param[in]	disable	true if called from COMMIT/ROLLBACK method */
  static void enter(trx_t *trx, bool disable) {
    if (srv_read_only_mode) {
      return;
    }

    ut_ad(!is_async_rollback(trx));

    /* If it hasn't already been marked for async rollback.
    and it will be committed/rolled back. */
    if (disable) {
      trx_mutex_enter(trx);
      if (!is_forced_rollback(trx) && is_started(trx) &&
          !trx_is_autocommit_non_locking(trx)) {
        ut_ad(trx->killed_by == 0);

        /* This transaction has crossed the point of
        no return and cannot be rolled back
        asynchronously now. It must commit or rollback
        synhronously. */

        trx->in_innodb |= TRX_FORCE_ROLLBACK_DISABLE;
      }
      trx_mutex_exit(trx);
    }

    /* Avoid excessive mutex acquire/release */
    ++trx->in_depth;

    /* If trx->in_depth is greater than 1 then
    transaction is already in InnoDB. */
    if (trx->in_depth > 1) {
      return;
    }

    trx_mutex_enter(trx);

    wait(trx);

    ut_ad((trx->in_innodb & TRX_FORCE_ROLLBACK_MASK) == 0);

    ++trx->in_innodb;

    trx_mutex_exit(trx);
  }

  /**
  Note that we are exiting InnoDB code */
  static void exit(trx_t *trx) {
    if (srv_read_only_mode) {
      return;
    }

    /* Avoid excessive mutex acquire/release */

    ut_ad(trx->in_depth > 0);

    --trx->in_depth;

    if (trx->in_depth > 0) {
      return;
    }

    trx_mutex_enter(trx);

    ut_ad((trx->in_innodb & TRX_FORCE_ROLLBACK_MASK) > 0);

    --trx->in_innodb;

    trx_mutex_exit(trx);
  }

  /*
  @return true if it is a forced rollback, asynchronously */
  static bool is_forced_rollback(const trx_t *trx) {
    ut_ad(trx_mutex_own(trx));

    return ((trx->in_innodb & TRX_FORCE_ROLLBACK)) > 0;
  }

  /**
  Wait for the asynchronous rollback to complete, if it is in progress */
  static void wait(trx_t *trx) {
    ut_ad(trx_mutex_own(trx));

    ulint loop_count = 0;
    /* start with optimistic sleep time - 20 micro seconds. */
    ulint sleep_time = 20;

    while (is_forced_rollback(trx)) {
      /* Wait for the async rollback to complete */

      trx_mutex_exit(trx);

      loop_count++;
      /* If the wait is long, don't hog the cpu. */
      if (loop_count < 100) {
        /* 20 microseconds */
        sleep_time = 20;
      } else if (loop_count < 1000) {
        /* 1 millisecond */
        sleep_time = 1000;
      } else {
        /* 100 milliseconds */
        sleep_time = 100000;
      }

      os_thread_sleep(sleep_time);

      trx_mutex_enter(trx);
    }
  }

  /**
  @return true if transaction is started */
  static bool is_started(const trx_t *trx) {
    ut_ad(trx_mutex_own(trx));

    return (trx_is_started(trx));
  }

 private:
  /**
  Transaction instance crossing the handler boundary from the Server. */
  trx_t *m_trx;
};


/**
@param trx		Get the active view for this transaction, if one exists
@return the transaction's read view or NULL if one not assigned. */
UNIV_INLINE
ReadView *trx_get_read_view(trx_t *trx) {
  return (!MVCC::is_view_active(trx->read_view) ? NULL : trx->read_view);
}

/**
@param trx		Get the active view for this transaction, if one exists
@return the transaction's read view or NULL if one not assigned. */
UNIV_INLINE
const ReadView *trx_get_read_view(const trx_t *trx) {
  return (!MVCC::is_view_active(trx->read_view) ? NULL : trx->read_view);
}

#endif /* !UNIV_HOTBACKUP */

#endif
