#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

/** Disconnect a prepared transaction from MySQL.
@param[in,out]	trx	transaction */
void trx_disconnect_prepared(trx_t *trx);
