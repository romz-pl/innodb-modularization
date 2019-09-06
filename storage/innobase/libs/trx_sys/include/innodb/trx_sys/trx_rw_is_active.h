#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

/** Checks if a rw transaction with the given id is active. If the caller is
not holding trx_sys->mutex, the transaction may already have been committed.
@param[in]	trx_id		trx id of the transaction
@param[in]	corrupt		NULL or pointer to a flag that will be set if
                                corrupt
@param[in]	do_ref_count	if true then increment the trx_t::n_ref_count
@return transaction instance if active, or NULL; */
trx_t *trx_rw_is_active(trx_id_t trx_id, ibool *corrupt, bool do_ref_count);
