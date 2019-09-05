#include <innodb/trx_trx/trx_create_low.h>

#include <innodb/memory/mem_heap_create.h>
#include <innodb/trx_trx/trx_pools.h>
#include <innodb/trx_trx/trx_t.h>
#include <innodb/trx_types/flags.h>
#include <innodb/trx_types/trx_state_t.h>
#include <innodb/vector/vector.h>
#include <innodb/trx_trx/assert_trx_is_free.h>


/** @return a trx_t instance from trx_pools. */
trx_t *trx_create_low() {
  trx_t *trx = trx_pools->get();

  assert_trx_is_free(trx);

  mem_heap_t *heap;
  ib_alloc_t *alloc;

  /* We just got trx from pool, it should be non locking */
  ut_ad(trx->will_lock == 0);

  trx->api_trx = false;

  trx->api_auto_commit = false;

  trx->read_write = true;

  /* Background trx should not be forced to rollback,
  we will unset the flag for user trx. */
  trx->in_innodb |= TRX_FORCE_ROLLBACK_DISABLE;

  /* Trx state can be TRX_STATE_FORCED_ROLLBACK if
  the trx was forced to rollback before it's reused.*/
  trx->state = TRX_STATE_NOT_STARTED;

  heap = mem_heap_create(sizeof(ib_vector_t) + sizeof(void *) * 8);

  alloc = ib_heap_allocator_create(heap);

  /* Remember to free the vector explicitly in trx_free(). */
  trx->autoinc_locks = ib_vector_create(alloc, sizeof(void **), 4);

  /* Should have been either just initialized or .clear()ed by
  trx_free(). */
  ut_a(trx->mod_tables.size() == 0);

  return (trx);
}
