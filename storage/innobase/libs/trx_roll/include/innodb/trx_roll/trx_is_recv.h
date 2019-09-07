#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

/** Determines if this transaction is rolling back an incomplete transaction
 in crash recovery.
 @return true if trx is an incomplete transaction that is being rolled
 back in crash recovery */
ibool trx_is_recv(const trx_t *trx); /*!< in: transaction */
