#include <innodb/trx_trx/trx_is_started.h>

#include <innodb/trx_trx/trx_t.h>

/**
Check if transaction is started.
@param[in] trx		Transaction whose state we need to check
@return true if transaction is in state started */
bool trx_is_started(const trx_t *trx) {
  return (trx->state != TRX_STATE_NOT_STARTED &&
          trx->state != TRX_STATE_FORCED_ROLLBACK);
}
