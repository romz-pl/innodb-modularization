#include <innodb/trx_trx/trx_release_reference.h>

#include <innodb/trx_trx/trx_t.h>
#include <innodb/trx_trx/trx_mutex_enter.h>
#include <innodb/trx_trx/trx_mutex_exit.h>

/**
Release the transaction. Decrease the reference count.
@param trx Transaction that is being released */
void trx_release_reference(trx_t *trx) {
  trx_mutex_enter(trx);

  ut_ad(trx->n_ref > 0);
  --trx->n_ref;

  trx_mutex_exit(trx);
}
