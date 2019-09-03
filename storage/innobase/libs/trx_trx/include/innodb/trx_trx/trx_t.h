#pragma once

#include <innodb/univ/univ.h>

#include <innodb/lst/lst.h>
#include <innodb/trx_types/TrxMutex.h>
#include <innodb/trx_types/trx_state_t.h>
#include <innodb/trx_trx/trx_lock_t.h>
#include <innodb/trx_trx/hit_list_t.h>
#include <innodb/thread/os_thread_id_t.h>
#include <innodb/trx_types/trx_dict_op_t.h>
#include <innodb/log_sn/lsn_t.h>
#include <innodb/error/dberr_t.h>
#include <innodb/trx_types/UndoMutex.h>
#include <innodb/trx_types/trx_savept_t.h>
#include <innodb/trx_trx/trx_rsegs_t.h>
#include <innodb/vector/vector.h>
#include <innodb/trx_trx/trx_mod_tables_t.h>

#include "sql/handler.h"

typedef ib_uint64_t doc_id_t;
typedef struct que_fork_t que_t;
struct sess_t;
class ReadView;
struct dict_index_t;
struct trx_named_savept_t;
struct fts_trx_t;
class FlushObserver;

struct trx_t {
  enum isolation_level_t {

    /** dirty read: non-locking SELECTs are performed so that we
    do not look at a possible earlier version of a record; thus
    they are not 'consistent' reads under this isolation level;
    otherwise like level 2 */
    READ_UNCOMMITTED,

    /** somewhat Oracle-like isolation, except that in range UPDATE
    and DELETE we must block phantom rows with next-key locks;
    SELECT ... FOR UPDATE and ...  LOCK IN SHARE MODE only lock
    the index records, NOT the gaps before them, and thus allow
    free inserting; each consistent read reads its own snapshot */
    READ_COMMITTED,

    /** this is the default; all consistent reads in the same trx
    read the same snapshot; full next-key locking used in locking
    reads to block insertions into gaps */
    REPEATABLE_READ,

    /** all plain SELECTs are converted to LOCK IN SHARE MODE
    reads */
    SERIALIZABLE
  };

  TrxMutex mutex; /*!< Mutex protecting the fields
                  state and lock (except some fields
                  of lock, which are protected by
                  lock_sys->mutex) */

  bool owns_mutex; /*!< Set to the transaction that owns
                   the mutex during lock acquire and/or
                   release.

                   This is used to avoid taking the
                   trx_t::mutex recursively. */

  /* Note: in_depth was split from in_innodb for fixing a RO
  performance issue. Acquiring the trx_t::mutex for each row
  costs ~3% in performance. It is not required for correctness.
  Therefore we increment/decrement in_depth without holding any
  mutex. The assumption is that the Server will only ever call
  the handler from one thread. This is not true for kill_connection.
  Therefore in innobase_kill_connection. We don't increment this
  counter via TrxInInnoDB. */

  ib_uint32_t in_depth; /*!< Track nested TrxInInnoDB
                        count */

  ib_uint32_t in_innodb; /*!< if the thread is executing
                         in the InnoDB context count > 0. */

  bool abort; /*!< if this flag is set then
              this transaction must abort when
              it can */

  trx_id_t id; /*!< transaction id */

  trx_id_t no; /*!< transaction serialization number:
               max trx id shortly before the
               transaction is moved to
               COMMITTED_IN_MEMORY state.
               Protected by trx_sys_t::mutex
               when trx->in_rw_trx_list. Initially
               set to TRX_ID_MAX. */

  /** State of the trx from the point of view of concurrency control
  and the valid state transitions.

  Possible states:

  TRX_STATE_NOT_STARTED
  TRX_STATE_FORCED_ROLLBACK
  TRX_STATE_ACTIVE
  TRX_STATE_PREPARED
  TRX_STATE_COMMITTED_IN_MEMORY (alias below COMMITTED)

  Valid state transitions are:

  Regular transactions:
  * NOT_STARTED -> ACTIVE -> COMMITTED -> NOT_STARTED

  Auto-commit non-locking read-only:
  * NOT_STARTED -> ACTIVE -> NOT_STARTED

  XA (2PC):
  * NOT_STARTED -> ACTIVE -> PREPARED -> COMMITTED -> NOT_STARTED

  Recovered XA:
  * NOT_STARTED -> PREPARED -> COMMITTED -> (freed)

  XA (2PC) (shutdown or disconnect before ROLLBACK or COMMIT):
  * NOT_STARTED -> PREPARED -> (freed)

  Disconnected XA can become recovered:
  * ... -> ACTIVE -> PREPARED (connected) -> PREPARED (disconnected)
  Disconnected means from mysql e.g due to the mysql client disconnection.
  Latching and various transaction lists membership rules:

  XA (2PC) transactions are always treated as non-autocommit.

  Transitions to ACTIVE or NOT_STARTED occur when
  !in_rw_trx_list (no trx_sys->mutex needed).

  Autocommit non-locking read-only transactions move between states
  without holding any mutex. They are !in_rw_trx_list.

  All transactions, unless they are determined to be ac-nl-ro,
  explicitly tagged as read-only or read-write, will first be put
  on the read-only transaction list. Only when a !read-only transaction
  in the read-only list tries to acquire an X or IX lock on a table
  do we remove it from the read-only list and put it on the read-write
  list. During this switch we assign it a rollback segment.

  When a transaction is NOT_STARTED, it can be in_mysql_trx_list if
  it is a user transaction. It cannot be in rw_trx_list.

  ACTIVE->PREPARED->COMMITTED is only possible when trx->in_rw_trx_list.
  The transition ACTIVE->PREPARED is protected by trx_sys->mutex.

  ACTIVE->COMMITTED is possible when the transaction is in
  rw_trx_list.

  Transitions to COMMITTED are protected by both lock_sys->mutex
  and trx->mutex.

  NOTE: Some of these state change constraints are an overkill,
  currently only required for a consistent view for printing stats.
  This unnecessarily adds a huge cost for the general case. */

  trx_state_t state;

  /* If set, this transaction should stop inheriting (GAP)locks.
  Generally set to true during transaction prepare for RC or lower
  isolation, if requested. Needed for replication replay where
  we don't want to get blocked on GAP locks taken for protecting
  concurrent unique insert or replace operation. */
  bool skip_lock_inheritance;

  ReadView *read_view; /*!< consistent read view used in the
                       transaction, or NULL if not yet set */

  UT_LIST_NODE_T(trx_t)
  trx_list; /*!< list of transactions;
            protected by trx_sys->mutex. */
  UT_LIST_NODE_T(trx_t)
  no_list; /*!< Required during view creation
           to check for the view limit for
           transactions that are committing */

  trx_lock_t lock;   /*!< Information about the transaction
                     locks and state. Protected by
                     trx->mutex or lock_sys->mutex
                     or both */
  bool is_recovered; /*!< 0=normal transaction,
                     1=recovered, must be rolled back,
                     protected by trx_sys->mutex when
                     trx->in_rw_trx_list holds */

  hit_list_t hit_list; /*!< List of transactions to kill,
                       when a high priority transaction
                       is blocked on a lock wait. */

  os_thread_id_t killed_by; /*!< The thread ID that wants to
                            kill this transaction asynchronously.
                            This is required because we recursively
                            enter the handlerton methods and need
                            to distinguish between the kill thread
                            and the transaction thread.

                            Note: We need to be careful w.r.t the
                            Thread Pool. The thread doing the kill
                            should not leave InnoDB between the
                            mark and the actual async kill because
                            the running thread can change. */

  /* These fields are not protected by any mutex. */
  const char *op_info;   /*!< English text describing the
                         current operation, or an empty
                         string */
  ulint isolation_level; /*!< TRX_ISO_REPEATABLE_READ, ... */
  bool check_foreigns;   /*!< normally TRUE, but if the user
                         wants to suppress foreign key checks,
                         (in table imports, for example) we
                         set this FALSE */
  /*------------------------------*/
  /* MySQL has a transaction coordinator to coordinate two phase
  commit between multiple storage engines and the binary log. When
  an engine participates in a transaction, it's responsible for
  registering itself using the trans_register_ha() API. */
  bool is_registered; /* This flag is set to true after the
                      transaction has been registered with
                      the coordinator using the XA API, and
                      is set to false  after commit or
                      rollback. */
  /*------------------------------*/
  bool check_unique_secondary;
  /*!< normally TRUE, but if the user
  wants to speed up inserts by
  suppressing unique key checks
  for secondary indexes when we decide
  if we can use the insert buffer for
  them, we set this FALSE */
  bool flush_log_later;      /* In 2PC, we hold the
                             prepare_commit mutex across
                             both phases. In that case, we
                             defer flush of the logs to disk
                             until after we release the
                             mutex. */
  bool must_flush_log_later; /*!< this flag is set to TRUE in
                        trx_commit() if flush_log_later was
                        TRUE, and there were modifications by
                        the transaction; in that case we must
                        flush the log in
                        trx_commit_complete_for_mysql() */
  ulint duplicates;          /*!< TRX_DUP_IGNORE | TRX_DUP_REPLACE */
  bool has_search_latch;
  /*!< TRUE if this trx has latched the
  search system latch in S-mode.
  This now can only be true in
  row_search_mvcc, the btr search latch
  must has been released before exiting,
  and this flag would be set to false */
  trx_dict_op_t dict_operation; /**< @see enum trx_dict_op_t */

  bool ddl_operation;  /*!< True if this trx involves dd table
                        change */
  bool ddl_must_flush; /*!< True if this trx involves dd table
                       change, and must flush */
  bool in_truncate;    /* This trx is doing truncation */

  /* Fields protected by the srv_conc_mutex. */
  bool declared_to_be_inside_innodb;
  /*!< this is TRUE if we have declared
  this transaction in
  srv_conc_enter_innodb to be inside the
  InnoDB engine */
  ib_uint32_t n_tickets_to_enter_innodb;
  /*!< this can be > 0 only when
  declared_to_... is TRUE; when we come
  to srv_conc_innodb_enter, if the value
  here is > 0, we decrement this by 1 */
  ib_uint32_t dict_operation_lock_mode;
  /*!< 0, RW_S_LATCH, or RW_X_LATCH:
  the latch mode trx currently holds
  on dict_operation_lock. Protected
  by dict_operation_lock. */

  time_t start_time; /*!< time the state last time became
                     TRX_STATE_ACTIVE */

  /** Weight/Age of the transaction in the record lock wait queue. */
  int32_t age;

  /** For tracking if Weight/age has been updated. */
  uint64_t age_updated;

  lsn_t commit_lsn; /*!< lsn at the time of the commit */

  /*------------------------------*/
  THD *mysql_thd; /*!< MySQL thread handle corresponding
                  to this trx, or NULL */

  const char *mysql_log_file_name;
  /*!< if MySQL binlog is used, this field
  contains a pointer to the latest file
  name; this is NULL if binlog is not
  used */
  int64_t mysql_log_offset;
  /*!< if MySQL binlog is used, this
  field contains the end offset of the
  binlog entry */
  /*------------------------------*/
  ib_uint32_t n_mysql_tables_in_use; /*!< number of Innobase tables
                              used in the processing of the current
                              SQL statement in MySQL */
  ib_uint32_t mysql_n_tables_locked;
  /*!< how many tables the current SQL
  statement uses, except those
  in consistent read */
  /*------------------------------*/
#ifdef UNIV_DEBUG
  /** The following two fields are mutually exclusive. */
  /* @{ */

  bool in_rw_trx_list; /*!< true if in trx_sys->rw_trx_list */
                       /* @} */
#endif                 /* UNIV_DEBUG */
  UT_LIST_NODE_T(trx_t)
  mysql_trx_list; /*!< list of transactions created for
                  MySQL; protected by trx_sys->mutex */
#ifdef UNIV_DEBUG
  bool in_mysql_trx_list;
  /*!< true if in
  trx_sys->mysql_trx_list */
#endif /* UNIV_DEBUG */
  /*------------------------------*/
  dberr_t error_state;             /*!< 0 if no error, otherwise error
                                   number; NOTE That ONLY the thread
                                   doing the transaction is allowed to
                                   set this field: this is NOT protected
                                   by any mutex */
  const dict_index_t *error_index; /*!< if the error number indicates a
                                   duplicate key error, a pointer to
                                   the problematic index is stored here */
  ulint error_key_num;             /*!< if the index creation fails to a
                                   duplicate key error, a mysql key
                                   number of that index is stored here */
  sess_t *sess;                    /*!< session of the trx, NULL if none */
  que_t *graph;                    /*!< query currently run in the session,
                                   or NULL if none; NOTE that the query
                                   belongs to the session, and it can
                                   survive over a transaction commit, if
                                   it is a stored procedure with a COMMIT
                                   WORK statement, for instance */
  /*------------------------------*/
  UT_LIST_BASE_NODE_T(trx_named_savept_t)
  trx_savepoints; /*!< savepoints set with SAVEPOINT ...,
                  oldest first */
  /*------------------------------*/
  UndoMutex undo_mutex; /*!< mutex protecting the fields in this
                        section (down to undo_no_arr), EXCEPT
                        last_sql_stat_start, which can be
                        accessed only when we know that there
                        cannot be any activity in the undo
                        logs! */
  undo_no_t undo_no;    /*!< next undo log record number to
                        assign; since the undo log is
                        private for a transaction, this
                        is a simple ascending sequence
                        with no gaps; thus it represents
                        the number of modified/inserted
                        rows in a transaction */
  space_id_t undo_rseg_space;
  /*!< space id where last undo record
  was written */
  trx_savept_t last_sql_stat_start;
  /*!< undo_no when the last sql statement
  was started: in case of an error, trx
  is rolled back down to this undo
  number; see note at undo_mutex! */
  trx_rsegs_t rsegs;    /* rollback segments for undo logging */
  undo_no_t roll_limit; /*!< least undo number to undo during
                        a partial rollback; 0 otherwise */
#ifdef UNIV_DEBUG
  bool in_rollback;   /*!< true when the transaction is
                      executing a partial or full rollback */
#endif                /* UNIV_DEBUG */
  ulint pages_undone; /*!< number of undo log pages undone
                      since the last undo log truncation */
  /*------------------------------*/
  ulint n_autoinc_rows;       /*!< no. of AUTO-INC rows required for
                              an SQL statement. This is useful for
                              multi-row INSERTs */
  ib_vector_t *autoinc_locks; /* AUTOINC locks held by this
                              transaction. Note that these are
                              also in the lock list trx_locks. This
                              vector needs to be freed explicitly
                              when the trx instance is destroyed.
                              Protected by lock_sys->mutex. */
  /*------------------------------*/
  bool read_only;        /*!< true if transaction is flagged
                         as a READ-ONLY transaction.
                         if auto_commit && will_lock == 0
                         then it will be handled as a
                         AC-NL-RO-SELECT (Auto Commit Non-Locking
                         Read Only Select). A read only
                         transaction will not be assigned an
                         UNDO log. */
  bool auto_commit;      /*!< true if it is an autocommit */
  ib_uint32_t will_lock; /*!< Will acquire some locks. Increment
                         each time we determine that a lock will
                         be acquired by the MySQL layer. */
#ifndef UNIV_HOTBACKUP
  /*------------------------------*/
  fts_trx_t *fts_trx;       /*!< FTS information, or NULL if
                            transaction hasn't modified tables
                            with FTS indexes (yet). */
  doc_id_t fts_next_doc_id; /* The document id used for updates */
  /*------------------------------*/
  ib_uint32_t flush_tables; /*!< if "covering" the FLUSH TABLES",
                            count of tables being flushed. */

  /*------------------------------*/
  bool internal; /*!< true if it is a system/internal
                 transaction background task. Such
                 transactions are always treated as
                 read-write. */
                 /*------------------------------*/
#ifdef UNIV_DEBUG
  ulint start_line;       /*!< Track where it was started from */
  const char *start_file; /*!< Filename where it was started */
#endif                    /* UNIV_DEBUG */

  lint n_ref; /*!< Count of references, protected
              by trx_t::mutex. We can't release the
              locks nor commit the transaction until
              this reference is 0.  We can change
              the state to COMMITTED_IN_MEMORY to
              signify that it is no longer
              "active". */

  /** Version of this instance. It is incremented each time the
  instance is re-used in trx_start_low(). It is used to track
  whether a transaction has been restarted since it was tagged
  for asynchronous rollback. */
  ulint version;

  XID *xid;                    /*!< X/Open XA transaction
                               identification to identify a
                               transaction branch */
  trx_mod_tables_t mod_tables; /*!< List of tables that were modified
                               by this transaction */
#endif                         /* !UNIV_HOTBACKUP */
                               /*------------------------------*/
  bool api_trx;                /*!< trx started by InnoDB API */
  bool api_auto_commit;        /*!< automatic commit */
  bool read_write;             /*!< if read and write operation */

  /*------------------------------*/
  char *detailed_error;          /*!< detailed error message for last
                                 error, or empty. */
  FlushObserver *flush_observer; /*!< flush observer */

#ifdef UNIV_DEBUG
  bool is_dd_trx; /*!< True if the transaction is used for
                  doing Non-locking Read-only Read
                  Committed on DD tables */
#endif            /* UNIV_DEBUG */
  ulint magic_n;

  bool is_read_uncommitted() const {
    return (isolation_level == READ_UNCOMMITTED);
  }

  bool skip_gap_locks() const {
    switch (isolation_level) {
      case READ_UNCOMMITTED:
      case READ_COMMITTED:
        return (true);
      case REPEATABLE_READ:
      case SERIALIZABLE:
        return (false);
    }
    ut_ad(0);
    return (false);
  }

  bool allow_semi_consistent() const { return (skip_gap_locks()); }
};
