#include <innodb/trx_trx/trx_free_for_background.h>

#include <innodb/trx_trx/trx_validate_state_before_free.h>
#include <innodb/trx_trx/trx_free.h>

/** Free a transaction that was allocated by background or user threads.
@param[in,out]	trx	transaction object to free */
void trx_free_for_background(trx_t *trx) {
  trx_validate_state_before_free(trx);

  trx_free(trx);
}
