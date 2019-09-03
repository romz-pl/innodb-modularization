#include <innodb/trx_trx/trx_reference.h>

#include <innodb/trx_trx/trx_t.h>
#include <innodb/trx_trx/trx_mutex_enter.h>
#include <innodb/trx_trx/trx_state_eq.h>
#include <innodb/trx_trx/trx_mutex_exit.h>

/**
Increase the reference count. If the transaction is in state
TRX_STATE_COMMITTED_IN_MEMORY then the transaction is considered
committed and the reference count is not incremented.
@param trx Transaction that is being referenced
@param do_ref_count Increment the reference iff this is true
@return transaction instance if it is not committed */
trx_t *trx_reference(trx_t *trx, bool do_ref_count) {
  trx_mutex_enter(trx);

  if (trx_state_eq(trx, TRX_STATE_COMMITTED_IN_MEMORY)) {
    trx_mutex_exit(trx);
    trx = NULL;
  } else if (do_ref_count) {
    ut_ad(trx->n_ref >= 0);
    ++trx->n_ref;
    trx_mutex_exit(trx);
  } else {
    trx_mutex_exit(trx);
  }

  return (trx);
}
