#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

/** Starts a transaction for internal processing. */
void trx_start_internal_low(trx_t *trx); /*!< in/out: transaction */
