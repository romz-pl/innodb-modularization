#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

/** Free and initialize a transaction object instantiated during recovery.
@param[in,out]	trx	transaction object to free and initialize */
void trx_free_resurrected(trx_t *trx);
