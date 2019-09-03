#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

/**
Increase the reference count. If the transaction is in state
TRX_STATE_COMMITTED_IN_MEMORY then the transaction is considered
committed and the reference count is not incremented.
@param trx Transaction that is being referenced
@param do_ref_count Increment the reference iff this is true
@return transaction instance if it is not committed */
trx_t *trx_reference(trx_t *trx, bool do_ref_count);
