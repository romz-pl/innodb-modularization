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

#include <innodb/trx_trx/trx_get_read_view.h>
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

#include <innodb/trx_trx/trx_dummy_sess.h>
#include <innodb/trx_trx/trx_erase_lists.h>
#include <innodb/trx_trx/flags.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/trx_trx/trx_set_flush_observer.h>
#include <innodb/trx_trx/trx_set_detailed_error.h>
#include <innodb/trx_trx/trx_set_detailed_error_from_file.h>


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

#include <innodb/trx_trx/TRX_WEIGHT.h>


/** Compares the "weight" (or size) of two transactions. Transactions that
 have edited non-transactional tables are considered heavier than ones
 that have not.
 @return true if weight(a) >= weight(b) */
bool trx_weight_ge(const trx_t *a,  /*!< in: the transaction to be compared */
                   const trx_t *b); /*!< in: the transaction to be compared */




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



#include <innodb/trx_trx/trx_immutable_id.h>
#include <innodb/trx_trx/trx_lock_wait_timeout_get.h>
#include <innodb/trx_trx/assert_trx_is_free.h>
#include <innodb/trx_trx/assert_trx_is_inactive.h>
#include <innodb/trx_trx/assert_trx_nonlocking_or_in_list.h>


#endif /* !UNIV_HOTBACKUP */







enum trx_rseg_type_t {
  TRX_RSEG_TYPE_NONE = 0, /*!< void rollback segment type. */
  TRX_RSEG_TYPE_REDO,     /*!< redo rollback segment. */
  TRX_RSEG_TYPE_NOREDO    /*!< non-redo rollback segment. */
};




#ifndef UNIV_HOTBACKUP


#include <innodb/trx_trx/trx_isolation_level.h>
#include <innodb/trx_trx/trx_is_started.h>



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




#include <innodb/trx_trx/TrxInInnoDB.h>




#endif /* !UNIV_HOTBACKUP */

#endif
