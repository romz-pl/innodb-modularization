#pragma once

#include <innodb/univ/univ.h>

#include <innodb/error/dberr_t.h>

struct trx_t;

/** Rollback a transaction used in MySQL.
 @return error code or DB_SUCCESS */
dberr_t trx_rollback_for_mysql(trx_t *trx); /*!< in/out: transaction */
