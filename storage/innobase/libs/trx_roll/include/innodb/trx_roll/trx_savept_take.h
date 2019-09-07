#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_types/trx_savept_t.h>

struct trx_t;

/** Returns a transaction savepoint taken at this point in time.
 @return savepoint */
trx_savept_t trx_savept_take(trx_t *trx); /*!< in: transaction */
