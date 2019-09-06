#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

/**
Kill all transactions that are blocking this transaction from acquiring locks.
@param[in,out] trx	High priority transaction */

void trx_kill_blocking(trx_t *trx);
