#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

/** At shutdown, frees a transaction object that is in the PREPARED state. */
void trx_free_prepared(trx_t *trx); /*!< in, own: trx object */
