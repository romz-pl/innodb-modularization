#pragma once

#include <innodb/univ/univ.h>

#include <innodb/error/dberr_t.h>

struct trx_t;

/** Does the transaction commit for MySQL.
 @return DB_SUCCESS or error number */
dberr_t trx_commit_for_mysql(trx_t *trx); /*!< in/out: transaction */
