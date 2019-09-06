#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

/** Starts a read-only transaction for internal processing.
@param[in,out] trx	transaction to be started */
void trx_start_internal_read_only_low(trx_t *trx);
