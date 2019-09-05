#include <innodb/trx_trx/trx_set_flush_observer.h>

#include <innodb/trx_trx/trx_t.h>

/** Set flush observer for the transaction
@param[in,out]	trx		transaction struct
@param[in]	observer	flush observer */
void trx_set_flush_observer(trx_t *trx, FlushObserver *observer) {
  trx->flush_observer = observer;
}
