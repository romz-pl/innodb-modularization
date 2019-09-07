#pragma once

#include <innodb/univ/univ.h>

#include <innodb/error/dberr_t.h>

struct trx_t;
struct trx_savept_t;

/** Rollback a transaction to a given savepoint or do a complete rollback.
 @return error code or DB_SUCCESS */
dberr_t trx_rollback_to_savepoint(
    trx_t *trx,            /*!< in: transaction handle */
    trx_savept_t *savept); /*!< in: pointer to savepoint undo number, if
                   partial rollback requested, or NULL for
                   complete rollback */
