#include <innodb/trx_trx/trx_state_eq.h>

#include <innodb/trx_trx/trx_t.h>

/** Determines if a transaction is in the given state.
 The caller must hold trx_sys->mutex, or it must be the thread
 that is serving a running transaction.
 A running RW transaction must be in trx_sys->rw_trx_list.
 @return true if trx->state == state */
bool trx_state_eq(const trx_t *trx,  /*!< in: transaction */
                  trx_state_t state) /*!< in: state */
{
#ifdef UNIV_DEBUG
  switch (trx->state) {
    case TRX_STATE_PREPARED:

      ut_ad(!trx_is_autocommit_non_locking(trx));
      return (trx->state == state);

    case TRX_STATE_ACTIVE:

      assert_trx_nonlocking_or_in_list(trx);
      return (state == trx->state);

    case TRX_STATE_COMMITTED_IN_MEMORY:

      check_trx_state(trx);
      return (state == trx->state);

    case TRX_STATE_NOT_STARTED:
    case TRX_STATE_FORCED_ROLLBACK:

      /* These states are not allowed for running transactions. */
      ut_a(state == TRX_STATE_NOT_STARTED ||
           state == TRX_STATE_FORCED_ROLLBACK);

      ut_ad(!trx->in_rw_trx_list);

      return (true);
  }
  ut_error;
#else  /* UNIV_DEBUG */
  return (trx->state == state);
#endif /* UNIV_DEBUG */
}
