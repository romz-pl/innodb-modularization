#include <innodb/trx_trx/trx_allocate_for_background.h>

#include <innodb/trx_trx/trx_create_low.h>
#include <innodb/trx_trx/trx_dummy_sess.h>
#include <innodb/trx_trx/trx_t.h>

/** Creates a transaction object for background operations by the master thread.
 @return own: transaction object */
trx_t *trx_allocate_for_background(void) {
  trx_t *trx;

  trx = trx_create_low();

  trx->sess = trx_dummy_sess;

  return (trx);
}
