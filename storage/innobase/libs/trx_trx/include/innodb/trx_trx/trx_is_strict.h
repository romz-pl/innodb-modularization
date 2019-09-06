#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

/** Determines if the currently running transaction is in strict mode.
 @return true if strict */
ibool trx_is_strict(trx_t *trx); /*!< in: transaction */
