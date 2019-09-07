#pragma once

#include <innodb/univ/univ.h>

#include <innodb/error/dberr_t.h>

struct trx_t;

/** Rolls back a transaction back to a named savepoint. Modifications after the
 savepoint are undone but InnoDB does NOT release the corresponding locks
 which are stored in memory. If a lock is 'implicit', that is, a new inserted
 row holds a lock where the lock information is carried by the trx id stored in
 the row, these locks are naturally released in the rollback. Savepoints which
 were set after this savepoint are deleted.
 @return if no savepoint of the name found then DB_NO_SAVEPOINT,
 otherwise DB_SUCCESS */
dberr_t trx_rollback_to_savepoint_for_mysql(
    trx_t *trx,                      /*!< in: transaction handle */
    const char *savepoint_name,      /*!< in: savepoint name */
    int64_t *mysql_binlog_cache_pos) /*!< out: the MySQL binlog cache
                                     position corresponding to this
                                     savepoint; MySQL needs this
                                     information to remove the
                                     binlog entries of the queries
                                     executed after the savepoint */
    MY_ATTRIBUTE((warn_unused_result));
