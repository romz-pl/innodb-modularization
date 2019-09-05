#include <innodb/trx_trx/trx_free_resurrected.h>

#include <innodb/trx_trx/trx_free.h>
#include <innodb/trx_trx/trx_init.h>
#include <innodb/trx_trx/trx_validate_state_before_free.h>

/** Free and initialize a transaction object instantiated during recovery.
@param[in,out]	trx	transaction object to free and initialize */
void trx_free_resurrected(trx_t *trx) {
  trx_validate_state_before_free(trx);

  trx_init(trx);

  trx_free(trx);
}
