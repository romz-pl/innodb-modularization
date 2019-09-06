#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

/** Free a transaction object for MySQL.
@param[in,out]	trx	transaction */
void trx_free_for_mysql(trx_t *trx);
