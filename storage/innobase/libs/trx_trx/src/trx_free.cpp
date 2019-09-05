#include <innodb/trx_trx/trx_free.h>

#include <innodb/trx_trx/assert_trx_is_free.h>
#include <innodb/trx_trx/trx_t.h>
#include <innodb/trx_trx/trx_pools.h>
#include <innodb/vector/vector.h>

/**
Release a trx_t instance back to the pool.
@param trx the instance to release. */
void trx_free(trx_t *&trx) {
  assert_trx_is_free(trx);

  trx->mysql_thd = 0;

  // FIXME: We need to avoid this heap free/alloc for each commit.
  if (trx->autoinc_locks != NULL) {
    ut_ad(ib_vector_is_empty(trx->autoinc_locks));
    /* We allocated a dedicated heap for the vector. */
    ib_vector_free(trx->autoinc_locks);
    trx->autoinc_locks = NULL;
  }

  trx->mod_tables.clear();

  ut_ad(trx->read_view == NULL);
  ut_ad(trx->is_dd_trx == false);

  /* trx locking state should have been reset before returning trx
  to pool */
  ut_ad(trx->will_lock == 0);

  trx_pools->mem_free(trx);

  trx = NULL;
}
