#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

/** Disconnect a transaction from MySQL.
@param[in,out]	trx	transaction */
void trx_disconnect_plain(trx_t *trx);
