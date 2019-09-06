#include <innodb/trx_trx/trx_free_prepared.h>

#include <innodb/trx_trx/trx_t.h>
#include <innodb/trx_trx/assert_trx_in_rw_list.h>
#include <innodb/trx_trx/trx_release_impl_and_expl_locks.h>
#include <innodb/trx_trx/trx_free.h>
#include <innodb/trx_trx/trx_state_eq.h>
#include <innodb/trx_types/flags.h>

void trx_undo_free_prepared(trx_t *trx);
void lock_trx_lock_list_init(trx_lock_list_t *lock_list);

/** At shutdown, frees a transaction object that is in the PREPARED state. */
void trx_free_prepared(trx_t *trx) /*!< in, own: trx object */
{
  ut_a(trx_state_eq(trx, TRX_STATE_PREPARED));
  ut_a(trx->magic_n == TRX_MAGIC_N);

  assert_trx_in_rw_list(trx);

  trx_release_impl_and_expl_locks(trx, false);
  trx_undo_free_prepared(trx);

  ut_ad(!trx->in_rw_trx_list);
  ut_a(!trx->read_only);

  trx->state = TRX_STATE_NOT_STARTED;

  /* Undo trx_resurrect_table_locks(). */
  lock_trx_lock_list_init(&trx->lock.trx_locks);

  trx_free(trx);
}
