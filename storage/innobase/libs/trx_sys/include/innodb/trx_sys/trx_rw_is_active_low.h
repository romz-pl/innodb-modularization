#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

/** Checks if a rw transaction with the given id is active.
@param[in]	trx_id		trx id of the transaction
@param[in]	corrupt		NULL or pointer to a flag that will be set if
                                corrupt
@return transaction instance if active, or NULL */
trx_t *trx_rw_is_active_low(trx_id_t trx_id, ibool *corrupt);
