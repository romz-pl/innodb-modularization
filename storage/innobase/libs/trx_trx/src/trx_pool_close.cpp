#include <innodb/trx_trx/trx_pool_close.h>

#include <innodb/trx_trx/trx_pools.h>

/** Destroy the trx_t pool */
void trx_pool_close() {
  UT_DELETE(trx_pools);

  trx_pools = 0;
}
