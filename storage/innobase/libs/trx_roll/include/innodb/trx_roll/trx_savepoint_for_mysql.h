#pragma once

#include <innodb/univ/univ.h>

#include <innodb/error/dberr_t.h>

struct trx_t;

/** Creates a named savepoint. If the transaction is not yet started, starts it.
 If there is already a savepoint of the same name, this call erases that old
 savepoint and replaces it with a new. Savepoints are deleted in a transaction
 commit or rollback.
 @return always DB_SUCCESS */
dberr_t trx_savepoint_for_mysql(
    trx_t *trx,                 /*!< in: transaction handle */
    const char *savepoint_name, /*!< in: savepoint name */
    int64_t binlog_cache_pos);  /*!< in: MySQL binlog cache
                                position corresponding to this
                                connection at the time of the
                                savepoint */
