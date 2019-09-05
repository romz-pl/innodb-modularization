#include <innodb/trx_trx/trx_pool_init.h>

#include <innodb/trx_trx/trx_pools.h>

/** Size of on trx_t pool in bytes. */
static const ulint MAX_TRX_BLOCK_SIZE = 1024 * 1024 * 4;

/** Create the trx_t pool */
void trx_pool_init() {
  trx_pools = UT_NEW_NOKEY(trx_pools_t(MAX_TRX_BLOCK_SIZE));

  ut_a(trx_pools != 0);
}
