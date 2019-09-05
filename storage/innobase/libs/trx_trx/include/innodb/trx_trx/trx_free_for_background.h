#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

/** Free a transaction that was allocated by background or user threads.
@param[in,out]	trx	transaction object to free */
void trx_free_for_background(trx_t *trx);
