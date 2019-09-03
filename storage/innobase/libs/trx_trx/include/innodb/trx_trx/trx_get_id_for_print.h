#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

/** Retreieves the transaction ID.
In a given point in time it is guaranteed that IDs of the running
transactions are unique. The values returned by this function for readonly
transactions may be reused, so a subsequent RO transaction may get the same ID
as a RO transaction that existed in the past. The values returned by this
function should be used for printing purposes only.
@param[in]	trx	transaction whose id to retrieve
@return transaction id */
trx_id_t trx_get_id_for_print(const trx_t *trx);
