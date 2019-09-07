#pragma once

#include <innodb/univ/univ.h>

#include <innodb/error/dberr_t.h>

struct trx_t;

/** Releases a named savepoint. Savepoints which
 were set after this savepoint are deleted.
 @return if no savepoint of the name found then DB_NO_SAVEPOINT,
 otherwise DB_SUCCESS */
dberr_t trx_release_savepoint_for_mysql(
    trx_t *trx,                 /*!< in: transaction handle */
    const char *savepoint_name) /*!< in: savepoint name */
    MY_ATTRIBUTE((warn_unused_result));
