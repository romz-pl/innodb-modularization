#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

/**
@param[in] requestor	Transaction requesting the lock
@param[in] holder	Transaction holding the lock
@return the transaction that will be rolled back, null don't care */
const trx_t *trx_arbitrate(const trx_t *requestor, const trx_t *holder);
