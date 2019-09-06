#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

/** Determines if the currently running transaction has been interrupted.
 @return true if interrupted */
ibool trx_is_interrupted(const trx_t *trx); /*!< in: transaction */
